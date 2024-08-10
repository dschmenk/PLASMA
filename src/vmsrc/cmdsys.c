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
 * TERM I/O structure for CLI interface
 */
struct termios org_tio;
/*
 * Symbol table
 */
#define SYMTBLSZ    1024
#define SYMSZ       16
#define MOD_ADDR        0x1000
#define DEF_CALL        0x0300
#define DEF_CALLSZ      0x0B00
#define DEF_ENTRYSZ     6
byte  symtbl[SYMTBLSZ];
byte *lastsym = symtbl;
uword heap    = 0x0200;
uword deftbl  = DEF_CALL;
uword lastdef = DEF_CALL;
/*
 * CMDSYS exports
 */
char *syslib_exp[] = {
    "CMDSYS",
    "MACHID",
    "PUTC",
    "PUTLN",
    "PUTS",
    "PUTI",
    "GETC",
    "GETS",
    "PUTB",
    "PUTH",
    "TOUPPER",
    "CALL",
    "SYSCALL",
    "HEAPMARK",
    "HEAPALLOCALLIGN",
    "HEAPALLOC",
    "HEAPRELEASE",
    "HEAPAVAIL",
    "MEMSET",
    "MEMCPY",
    "STRCPY",
    "STRCAT",
    "SEXT",
    "DIVMOD",
    "ISUGT",
    "ISUGE",
    "ISULT",
    "ISULE",
    0
};
#if 0
void call(M6502 *mpu, uword pc)
{
    unsigned int i, s;
    int a, b;
    char c, sz[64];

    if (show_state)
        printf("\nCall: %s\n", mem_6502[pc] ? syslib_exp[mem_6502[pc] - 1] : "BYTECODE");
    switch (mem_6502[pc++])
    {
        case 0: // BYTECODE in mem_6502
            vm_interp(mpu, mem_6502 + (mem_6502[pc] + (mem_6502[pc + 1] << 8)));
            break;
        case 1: // CMDSYS call
            printf("CMD call code!\n");
            break;
        case 2: // MACHID
            printf("MACHID call code!\n");
            break;
        case 3: // LIBRARY STDLIB::PUTC
            c = POP;
            if (c == 0x0D)
                c = '\n';
            putchar(c);
            break;
        case 4: // LIBRARY STDLIB::PUTNL
            putchar('\n');
            fflush(stdout);
            break;
        case 5: // LIBRARY STDLIB::PUTS
            s = POP;
            i = mem_6502[s++];
            while (i--)
            {
                c = mem_6502[s++];
                if (c == 0x0D)
                    c = '\n';
                putchar(c);
            }
            break;
        case 6: // LIBRARY STDLIB::PUTI
            i = POP;
            printf("%d", i);
            break;
        case 7: // LIBRARY STDLIB::GETC
            PUSH(getchar());
            break;
        case 8: // LIBRARY STDLIB::GETS
            c = POP;
            putchar(c);
            fgets(sz, 63, stdin);
            for (i = 0; sz[i]; i++)
                mem_6502[0x200 + i] = sz[i];
            mem_6502[0x200 + i] = 0;
            mem_6502[0x1FF] = i;
            PUSH(0x1FF);
            break;
        case 9: // LIBRARY STDLIB::PUTB
            i = UPOP;
            printf("%02X", i);
            break;
        case 10: // LIBRARY STDLIB::PUTH
            i = UPOP;
            printf("%04X", i);
            break;
        case 24: // LIBRARY CMDSYS::DIVMOD
            a = POP;
            b = POP;
            PUSH(b / a);
            PUSH(b % a);
            break;
        default:
            printf("\nUnimplemented call code:$%02X\n", mem_6502[pc - 1]);
            exit(1);
    }
}
#endif
/*
 * Simple I/O routines
 */
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
void pfail(const char *msg)
{
    fflush(stdout);
    perror(msg);
    exit(1);
}
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
            strcat(filename, ".mod");
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
            strcat(filename, ".mod");
            addr = mem_6502[CFFADest] | (mem_6502[CFFADest + 1] << 8);
            mpu->registers->a = load(addr, filename) - 1;
            break;
        default:
            {
                char state[64];
                fprintf(stderr, "Unimplemented CFFA function: %02X\n", mpu->registers->x);
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
int paused = 0;
unsigned char keypending = 0;
int bye(M6502 *mpu, uword addr, byte data) { exit(0); return 0; }
int cout(M6502 *mpu, uword addr, byte data)  { if (mpu->registers->a == 0x8D) putchar('\n'); putchar(mpu->registers->a & 0x7F); fflush(stdout); RTS; }
unsigned char keypressed(M6502 *mpu)
{
    unsigned char cin, cext[2];
    if (read(STDIN_FILENO, &cin, 1) > 0)
    {
        if (cin == 0x03) // CTRL-C
        {
            mpu->flags |= M6502_SingleStep;
            paused = 1;
        }
        if (cin == 0x1B) // Look for left arrow
        {
            if (read(STDIN_FILENO, cext, 2) == 2 && cext[0] == '[' && cext[1] == 'D')
            cin = 0x08;
        }
        keypending = cin | 0x80;
    }
    return keypending & 0x80;
}
unsigned char keyin(M6502 *mpu)
{
    unsigned char cin;

    if (!keypending)
        keypressed(mpu);
    cin = keypending;
    keypending = 0;
    return cin;
}
int rd6820kbdctl(M6502 *mpu, uword addr, byte data) { return keypressed(mpu); }
int rd6820vidctl(M6502 *mpu, uword addr, byte data) { return 0x00; }
int rd6820kbd(M6502 *mpu, uword addr, byte data)    { return keyin(mpu); }
int rd6820vid(M6502 *mpu, uword addr, byte data)    { return 0x80; }
int wr6820vid(M6502 *mpu, uword addr, byte data)    { if (data == 0x8D) putchar('\n'); putchar(data & 0x7F); fflush(stdout); return 0; }
int setTraps(M6502 *mpu)
{
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
    // Hook BRK to enter VM or native code
    //
    M6502_setCallback(mpu, call, VM_INLINE_ENTRY, vm_indef);
    M6502_setCallback(mpu, call, VM_EXT_ENTRY,    vm_exdef);
    M6502_setCallback(mpu, call, VM_NATV_ENTRY,   vm_natvdef);
    M6502_setCallback(mpu, call, 0x0000, vm_irq);
    //
    // Hook Apple 1 IINTERP entrypoint
    //
    M6502_setCallback(mpu, call, 0x0283, vm_indef);
    M6502_setCallback(mpu, call, 0x0293, vm_iidef);
    return 0;
}
/*
 * M6502_run() with trace/single step ability
 */
void M6502_exec(M6502 *mpu)
{
    char state[64];
    char insn[64];

    do
    {
        if (mpu->flags & M6502_SingleStep)
        {
            M6502_dump(mpu, state);
            M6502_disassemble(mpu, mpu->registers->pc, insn);
            printf("%s : %s\r\n", state, insn);
            if (paused || (keypressed(mpu) && keypending == 0x83))
            {
                keypending = 0;
                while (!keypressed(mpu));
                if (keypending == (0x80|'C'))
                    paused = 0;
                else if (keypending == (0x80|'Q'))
                    break;
                else if (keypending == 0x9B) // Escape (QUIT)
                    exit(-1);
                keypending = 0;
            }
        }
    } while (M6502_run(mpu) > 0);

}
void resetInput(void)
{
    tcsetattr(STDIN_FILENO, TCSANOW, &org_tio);
}
void setRawInput(void)
{
    struct termios termio;

    tcgetattr(STDIN_FILENO, &termio); // save current port settings
    memcpy(&org_tio, &termio, sizeof(struct termios));
    termio.c_cflag     = /*BAUDRATE | CRTSCTS |*/ CS8 | CLOCAL | CREAD;
    termio.c_iflag     = IGNPAR;
    termio.c_oflag     = 0;
    termio.c_lflag     = 0; // set input mode (non-canonical, no echo,...)
    termio.c_cc[VTIME] = 0; // inter-character timer unused
    termio.c_cc[VMIN]  = 0; // non-blocking read
    tcsetattr(STDIN_FILENO, TCSANOW, &termio);
    atexit(resetInput);
}
/*
 * Utility routines.
 *
 * A DCI string is one that has the high bit set for every character except the last.
 * More space efficient than C or Pascal strings.
 */
int dcitos(byte *dci, char *str)
{
    int len = 0;
    do
        str[len] = tolower(*dci & 0x7F);
    while ((len++ < 16) && (*dci++ & 0x80));
    str[len] = 0;
    return len;
}
int stodci(char *str, byte *dci)
{
    int len = 0;
    do
        dci[len] = toupper(*str) | 0x80;
    while (*str++ && (len++ < 16));
    dci[len - 1] &= 0x7F;
    return len;
}
/*
 * Heap routines.
 */
uword avail_heap(void)
{
    uword vm_fp = UWORD_PTR(&mem_6502[FP]);
    return vm_fp - heap;
}
uword alloc_heap(int size)
{
    uword vm_fp = UWORD_PTR(&mem_6502[FP]);
    uword addr = heap;
    heap += size;
    if (heap >= vm_fp)
    {
        printf("Error: heap/frame collision.\n");
        exit (1);
    }
    return addr;
}
uword free_heap(int size)
{
    uword vm_fp = UWORD_PTR(&mem_6502[FP]);
    heap -= size;
    return vm_fp - heap;
}
uword mark_heap(void)
{
    return heap;
}
uword release_heap(uword newheap)
{
    uword vm_fp = UWORD_PTR(&mem_6502[FP]);
    heap = newheap;
    return vm_fp - heap;
}
/*
 * DCI table routines,
 */
void dump_tbl(byte *tbl)
{
    int len;
    byte *entbl;
    while (*tbl)
    {
        len = 0;
        while (*tbl & 0x80)
        {
            putchar(*tbl++ & 0x7F);
            len++;
        }
        putchar(*tbl++);
        putchar(':');
        while (len++ < 15)
            putchar(' ');
        printf("$%04X\n", tbl[0] | (tbl[1] << 8));
        tbl += 2;
    }
}
uword lookup_tbl(byte *dci, byte *tbl)
{
    char str[20];
    byte *match, *entry = tbl;
    while (*entry)
    {
        match = dci;
        while (*entry == *match)
        {
            if (!(*entry & 0x80))
                return entry[1] | (entry[2] << 8);
            entry++;
            match++;
        }
        while (*entry++ & 0x80);
        entry += 2;
    }
    dcitos(dci, str);
    printf("\nError: symbols %s not found in symbol table.\n", str);
    return 0;
}
uword add_tbl(byte *dci, int val, byte **last)
{
    while (*dci & 0x80)
        *(*last)++ = *dci++;
    *(*last)++ = *dci;
    *(*last)++ = val;
    *(*last)++ = val >> 8;
    return 0;
}
/*
 * Symbol table routines.
 */
void dump_sym(void)
{
    printf("\nSystem Symbol Table:\n");
    dump_tbl(symtbl);
}
uword lookup_sym(byte *sym)
{
    return lookup_tbl(sym, symtbl);
}
uword add_sym(byte *sym, int addr)
{
    return add_tbl(sym, addr, &lastsym);
}
/*
 * Module routines.
 */
uword defcall_add(int bank, int addr)
{
    mem_6502[lastdef]     = bank ? 2 : 1;
    mem_6502[lastdef + 1] = addr;
    mem_6502[lastdef + 2] = addr >> 8;
    return lastdef++;
}
uword def_lookup(byte *cdd, int defaddr)
{
    int i, calldef = 0;
    for (i = 0; cdd[i * 4] == 0x00; i++)
    {
        if ((cdd[i * 4 + 1] | (cdd[i * 4 + 2] << 8)) == defaddr)
        {
            calldef = cdd + i * 4 - mem_6502;
            break;
        }
    }
    return calldef;
}
uword extern_lookup(byte *esd, int index)
{
    byte *sym;
    char string[32];
    while (*esd)
    {
        sym = esd;
        esd += dcitos(esd, string);
        if ((esd[0] & 0x10) && (esd[1] == index))
            return lookup_sym(sym);
        esd += 3;
    }
    printf("\nError: extern index %d not found in ESD.\n", index);
    return 0;
}
int load_mod(M6502 *mpu, byte *mod)
{
    uword modsize, hdrlen, len, end, magic, bytecode, fixup, addr, sysflags, defcnt = 0, init = 0, modaddr = mark_heap();
    word modfix;
    byte *moddep, *rld, *esd, *cdd, *sym;
    byte header[128];
    int fd;
    char filename[48], string[17];

    dcitos(mod, filename);
    printf("Load module %s\n", filename);
    if (strlen(filename) < 8 || (filename[strlen(filename) - 7] != '#'))
        strcat(filename, ".mod");
    fd = open(filename, O_RDONLY, 0);
    if ((fd > 0) && (len = read(fd, header, 128)) > 0)
    {
        moddep  = header + 1;
        modsize = header[0] | (header[1] << 8);
        magic   = header[2] | (header[3] << 8);
        if (magic == 0x6502)
        {
            //
            // This is a relocatable bytecode module.
            //
            sysflags = header[4]  | (header[5]  << 8);
            bytecode = header[6]  | (header[7]  << 8);
            defcnt   = header[8]  | (header[9]  << 8);
            init     = header[10] | (header[11] << 8);
            moddep   = header + 12;
            //
            // Load module dependencies.
            //
            while (*moddep)
            {
                if (lookup_sym(moddep) == 0)
                {
                    if (fd)
                    {
                        close(fd);
                        fd = 0;
                    }
                    load_mod(mpu, moddep);
                }
                moddep += dcitos(moddep, string);
            }
            if (fd == 0)
            {
                fd = open(filename, O_RDONLY, 0);
                len = read(fd, header, 128);
            }
        }
        //
        // Alloc heap space for relocated module (data + bytecode).
        //
        moddep += 1;
        hdrlen  = moddep - header;
        len    -= hdrlen;
        modaddr = mark_heap();
        end     = modaddr + len;
        //
        // Read in remainder of module into memory for fixups.
        //
        memcpy(mem_6502 + modaddr, moddep, len);
        while ((len = read(fd, mem_6502 + end, 4096)) > 0)
            end += len;
        close(fd);
        //
        // Apply all fixups and symbol import/export.
        //
        modfix    = modaddr - hdrlen + 2; // - MOD_ADDR;
        bytecode += modfix - MOD_ADDR;
        end       = modaddr - hdrlen + modsize + 2;
        rld       = mem_6502 + end;       // Re-Locatable Directory
        esd       = rld;                  // Extern+Entry Symbol Directory
        while (*esd != 0x00)              // Scan to end of RLD
            esd += 4;
        esd++;
        cdd = rld;
        if (show_state)
        {
            //
            // Dump different parts of module.
            //
            printf("Module load addr: $%04X\n", modaddr);
            printf("Module size: %d\n", end - modaddr + hdrlen);
            printf("Module code+data size: %d\n", modsize);
            printf("Module magic: $%04X\n", magic);
            printf("Module sysflags: $%04X\n", sysflags);
            printf("Module bytecode: $%04X\n", bytecode);
            printf("Module def count: $%04X\n", defcnt);
            printf("Module init: $%04X\n", init ? init + modfix - MOD_ADDR : 0);
        }
        //
        // Add module to symbol table.
        //
        add_sym(mod, modaddr);
        //
        // Print out the Re-Location Dictionary.
        //
        if (show_state)
            printf("\nRe-Location Dictionary:\n");
        while (*rld)
        {
            if (rld[0] == 0x02)
            {
                if (show_state) printf("\tDEF               CODE");
                addr = rld[1] | (rld[2] << 8);
                addr += modfix - MOD_ADDR;
                rld[0] = 0; // Set call code to 0
                rld[1] = addr;
                rld[2] = addr >> 8;
                end = rld - mem_6502 + 4;
            }
            else
            {
                addr = rld[1] | (rld[2] << 8);
                if (addr > 12)
                {
                    addr += modfix;
                    if (rld[0] & 0x80)
                        fixup = (mem_6502[addr] | (mem_6502[addr + 1] << 8));
                    else
                        fixup = mem_6502[addr];
                    if (rld[0] & 0x10)
                    {
                        if (show_state) printf("\tEXTERN[$%02X]       ", rld[3]);
                        fixup += extern_lookup(esd, rld[3]);
                    }
                    else
                    {
                        fixup += modfix - MOD_ADDR;
                        if (fixup >= bytecode)
                        {
                            //
                            // Replace with call def dictionary.
                            //
                            if (show_state) printf("\tDEF[$%04X->", fixup);
                            fixup = def_lookup(cdd, fixup);
                            if (show_state) printf("$%04X] ", fixup);
                        }
                        else
                            if (show_state) printf("\tINTERN            ");
                    }
                    if (rld[0] & 0x80)
                    {
                        if (show_state) printf("WORD");
                        mem_6502[addr]     = fixup;
                        mem_6502[addr + 1] = fixup >> 8;
                    }
                    else
                    {
                        if (show_state) printf("BYTE");
                        mem_6502[addr] = fixup;
                    }
                }
                else
                {
                    if (show_state) printf("\tIGNORE (HDR)    ");
                }
            }
            if (show_state) printf("@$%04X\n", addr);
            rld += 4;
        }
        if (show_state) printf("\nExternal/Entry Symbol Directory:\n");
        while (*esd)
        {
            sym = esd;
            esd += dcitos(esd, string);
            if (esd[0] & 0x10)
            {
                if (show_state) printf("\tIMPORT %s[$%02X]\n", string, esd[1]);
            }
            else if (esd[0] & 0x08)
            {
                addr = esd[1] | (esd[2] << 8);
                addr += modfix - MOD_ADDR;
                if (show_state) printf("\tEXPORT %s@$%04X\n", string, addr);
                if (addr >= bytecode)
                    addr = def_lookup(cdd, addr);
                add_sym(sym, addr);
            }
            esd += 3;
        }
    }
    else
    {
        printf("Error: Unable to load module %s\n", filename);
        exit (1);
    }
    //
    // Reserve heap space for relocated module.
    //
    alloc_heap(end - modaddr);
    //
    //Call init routine.
    //
    if (init)
    {
        vm_interp(mpu, mem_6502 + init + modfix - MOD_ADDR);
//        release_heap(init + modfix - MOD_ADDR); // Free up init code
        init  = mem_6502[++mpu->registers->s + 0x100];
        init |= mem_6502[++mpu->registers->s + 0x100] << 8;
        return init;
    }
    return 0;
}
int main(int argc, char **argv)
{
    byte   dci[32];
    int    i;
    char  *interpfile = "a1plasma.bin";
    M6502 *mpu = M6502_new(0, mem_6502, 0);
    if (--argc)
    {
        argv++;
        while (argc && (*argv)[0] == '-')
        {
            if ((*argv)[1] == 's')
                show_state = 1;
            else if ((*argv)[1] == 't')
                mpu->flags |= M6502_SingleStep;
            argc--;
            argv++;
        }
        if (argc)
            interpfile = *argv;
    }
    //
    // Add default library.
    //
    for (i = 0; syslib_exp[i]; i++)
    {
        mem_6502[i] = i; // ???
        stodci(syslib_exp[i], dci);
        add_sym(dci, i+1);
    }
    //
    // Load module from command line
    //
    // PLVMC version
    //stodci(interpfile, dci);
    //if (show_state) dump_sym();
    //load_mod(dci);
    //if (show_state) dump_sym();
    // lib6502 version
    if (!load(0x280, interpfile))
        pfail(interpfile);
    setRawInput();
    setTraps(mpu);
    M6502_reset(mpu);
    mpu->registers->pc = 0x0280;
    mem_6502[ESP]      = ESTK_SIZE;
    mem_6502[FPL]      = 0x00;
    mem_6502[FPH]      = 0x80;
    mem_6502[PPL]      = 0x00;
    mem_6502[PPH]      = 0x80;
    M6502_exec(mpu);
    M6502_delete(mpu);
    return 0;
}
