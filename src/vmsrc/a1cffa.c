#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include "plvm.h"
/*
 * CFFA1 emulation
 */
#define CFFADest     0x00
#define CFFAFileName 0x02
#define CFFAOldName  0x04
#define CFFAFileType 0x06
#define CFFAAuxType  0x07
#define CFFAFileSize 0x09
#define CFFAEntryPtr 0x0B
int save(uword address, unsigned length, const char *path)
{
    FILE *file;
    int   count;
    if (!(file = fopen(path, "wb")))
        return 0;
    while ((count = fwrite(mem_6502 + address, 1, length, file)))
    {
        address += count;
        length -= count;
    }
    fclose(file);
    return 1;
}
int load(uword address, const char *path)
{
    FILE  *file;
    int    count;
    size_t max   = 0x10000 - address;
    if (!(file = fopen(path, "rb")))
        return 0;
    while ((count = fread(mem_6502 + address, 1, max, file)) > 0)
    {
        address += count;
        max -= count;
    }
    fclose(file);
    return 1;
}
char *strlower(char *strptr)
{
    int i;

    for (i = 0; strptr[i]; i++)
      strptr[i] = tolower(strptr[i]);
    return strptr;
}
int cffa1(M6502 *mpu, uword address, byte data)
{
    char *fileptr, filename[64];
    int addr;
    struct stat sbuf;

    switch (mpu->registers->x)
    {
        case 0x02:  // quit
            exit(0);
            break;
        case 0x14:  // find dir entry
            addr = mem_6502[CFFAFileName] | (mem_6502[CFFAFileName + 1] << 8);
            memset(filename, 0, 64);
            strncpy(filename, (char *)(mem_6502 + addr + 1), mem_6502[addr]);
            strlower(filename);
            //strcat(filename, ".mod");
            if (!(stat(filename, &sbuf)))
            {
                // DirEntry @ $9100
                mem_6502[CFFAEntryPtr]     = 0x00;
                mem_6502[CFFAEntryPtr + 1] = 0x91;
                mem_6502[0x9115] = sbuf.st_size;
                mem_6502[0x9116] = sbuf.st_size >> 8;
                mpu->registers->a = 0;
            }
            else
                mpu->registers->a = -1;
            break;
        case 0x22:  // load file
            addr = mem_6502[CFFAFileName] | (mem_6502[CFFAFileName + 1] << 8);
            memset(filename, 0, 64);
            strncpy(filename, (char *)(mem_6502 + addr + 1), mem_6502[addr]);
            strlower(filename);
            //strcat(filename, ".mod");
            addr = mem_6502[CFFADest] | (mem_6502[CFFADest + 1] << 8);
            mpu->registers->a = load(addr, filename) - 1;
            break;
        default:
            {
                char state[64];
                fprintf(stderr, "Unimplemented CFFA function: %02X\r\n", mpu->registers->x);
                M6502_dump(mpu, state);
                fflush(stdout);
                fprintf(stderr, "\nCFFA1 %s\n", state);
                pfail("ABORT");
            }
        break;
    }
    RTS;
}
/*
 * Character I/O emulation
 */
int bye(M6502 *mpu, uword addr, byte data) { exit(0); return 0; }
int cout(M6502 *mpu, uword addr, byte data)  { if (mpu->registers->a == 0x8D) putchar('\n'); putchar(mpu->registers->a & 0x7F); fflush(stdout); RTS; }
int rd6820kbdctl(M6502 *mpu, uword addr, byte data)
{
    unsigned char cin, cext[2];
    if (read(STDIN_FILENO, &cin, 1) > 0)
    {
        if (cin == 0x03) // CTRL-C
        {
            trace       = SINGLE_STEP;
            mpu->flags |= M6502_SingleStep;
            paused = 1;
        }
        if (cin == 0x1B) // Look for left arrow
        {
            if (read(STDIN_FILENO, cext, 2) == 2 && cext[0] == '[' && cext[1] == 'D')
            cin = 0x08;
        }
        keyqueue = cin | 0x80;
    }
    return keyqueue & 0x80;
}
int rd6820kbd(M6502 *mpu, uword addr, byte data)
{   unsigned char cin;

    if (!keyqueue)
        rd6820kbdctl(mpu, addr, data);
    cin = keyqueue;
    keyqueue = 0;
    return cin;
}
int rd6820vidctl(M6502 *mpu, uword addr, byte data) { return 0x00; }
int rd6820vid(M6502 *mpu, uword addr, byte data)    { return 0x80; }
int wr6820vid(M6502 *mpu, uword addr, byte data)    { if (data == 0x8D) putchar('\n'); putchar(data & 0x7F); fflush(stdout); return 0; }
int initApple1(M6502 *mpu, uword address, const char *path)
{
    M6502_setCallback(mpu, call, VM_IRQ_ENTRY, vm_irq);
    //
    // Apple 1 memory-mapped IO
    //
    M6502_setCallback(mpu, read,  0xD010, rd6820kbd);
    M6502_setCallback(mpu, read,  0xD011, rd6820kbdctl);
    M6502_setCallback(mpu, read,  0xD012, rd6820vid);
    M6502_setCallback(mpu, write, 0xD012, wr6820vid);
    M6502_setCallback(mpu, read,  0xD013, rd6820vidctl);
    //
    // CFFA1 and ROM calls
    //
    M6502_setCallback(mpu, call, 0x9000, bye);
    M6502_setCallback(mpu, call, 0x900C, cffa1);
    M6502_setCallback(mpu, call, 0xFFEF, cout);
    //
    // 32K RAM
    //
    mem_6502[FPL]    = 0x00; // Frame pointer = $8000
    mem_6502[FPH]    = 0x80;
    mem_6502[PPL]    = 0x00; // Pool pointer = $8000 (unused)
    mem_6502[PPH]    = 0x80;
    return load(address, path);
}
