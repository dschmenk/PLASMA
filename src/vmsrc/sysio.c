#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "plvm.h"

int nlfd[4];
char nlmask[4], nlchar[4];
/*
 * Console I/O
 */
void sysputln(M6502 *mpu)
{
    putchar('\r'); putchar('\n');
    fflush(stdout);
}
void sysputc(M6502 *mpu)
{
    char ch;
    //
    // Pop char and putchar it
    //
    PULL_ESTK(ch);
    switch (ch)
    {
        case '\r':
        case '\n':
            sysputln(mpu);
            break;
        default:
            putchar(ch);
    }
    fflush(stdout);
}
void sysputs(M6502 *mpu)
{
    uword strptr;
    int i;
    char ch;

    //
    // Pop string pointer off stack, copy into C string, and puts it
    //
    PULL_ESTK(strptr);
    for (i = 1; i <= mem_6502[strptr]; i++)
    {
        ch = mem_6502[strptr + i] & 0x7F;
        switch (ch)
        {
            case '\r':
            case '\n':
                sysputln(mpu);
                break;
            default:
                putchar(ch);
        }
    }
    fflush(stdout);
}
void sysputi(M6502 *mpu)
{
    word print;
    //
    // Pop int off stack, copy into C string, and print it
    //
    PULL_ESTK(print);
    printf("%d", print);
}
void sysputb(M6502 *mpu)
{
    uword prbyte;
    //
    // Pop byte off stack, copy into C string, and print it
    //
    PULL_ESTK(prbyte);
    printf("%02X", prbyte & 0xFF);
}
void sysputh(M6502 *mpu)
{
    uword prhex;
    //
    // Pop int off stack, copy into C string, and print it
    //
    PULL_ESTK(prhex);
    printf("%04X", prhex);
}
void syskeypressed(M6502 *mpu)
{
    PUSH_ESTK(keypressed(mpu));
}
void sysgetc(M6502 *mpu)
{
    PUSH_ESTK(keyin(mpu));
}
void sysgets(M6502 *mpu)
{
    uword strptr;
    int len;
    char cext[2], instr[256];
    byte cin;
    
    //
    // Push gets(), limiting it to 128 chars
    //
    PULL_ESTK(cext[0]);
    putchar(cext[0] & 0x7F);
    len = 0;
    fflush(stdout);
    do
    {
        switch (cin = keyin(mpu))
        {
            case 0x1B:
                if (read(STDIN_FILENO, cext, 2) == 2)
                    if (cext[0] == '[' && cext[1] == 'D') // Left arrow
                        cin = 0x08;
                    else
                        break;
                else
                    break;
            case 0x08: // BS
            case 0x7F: // BS
                if (len)
                {
                    putchar('\x08');
                    putchar(' ');
                    putchar('\x08');
                    len--;
                }
                break;
            default:
            if (cin >= ' ' || cin == 0x0D)
                putchar(instr[len++] = cin);
        }
        fflush(stdout);
    } while (cin != 0x0D && len < 128);
    sysputln(mpu);
    mem_6502[CMDLINE_STR] = len;
    memcpy(mem_6502 + CMDLINE_BUF, instr, len);
    PUSH_ESTK(CMDLINE_STR);
}
void sysecho(M6502 *mpu)
{
    int state;

    PULL_ESTK(state);
    fprintf(stderr, "CONIO:ECHO unimplemented!\r\n");
    PUSH_ESTK(0);
}
void syshome(M6502 *mpu)
{
    printf("\x1b[2J");
    PUSH_ESTK(0);
}
void sysgotoxy(M6502 *mpu)
{
    int x, y;

    PULL_ESTK(y);
    PULL_ESTK(x);
    printf("\x1b[%d;%df", y + 1, x + 1);
    PUSH_ESTK(0);
}
void sysviewport(M6502 *mpu)
{
    int x, y, w, h;

    PULL_ESTK(y);
    PULL_ESTK(x);
    PULL_ESTK(w);
    PULL_ESTK(h);
    fprintf(stderr, "CONIO:VIEWPORT unimplemented!\r\n");
    PUSH_ESTK(0);
}
void systexttype(M6502 *mpu)
{
    int mode;

    PULL_ESTK(mode);
    fprintf(stderr, "CONIO:TEXTYPE unimplemented!\r\n");
    PUSH_ESTK(0);
}
void systextmode(M6502 *mpu)
{
    int cols;

    PULL_ESTK(cols);
    printf("\x1b[49m\x1b[?25h");
    syshome(mpu);
}
void sysgrmode(M6502 *mpu)
{
    int split;

    PULL_ESTK(split);
    printf("\x1b[40m\x1b[?25l");
    syshome(mpu);
}
void sysgrcolor(M6502 *mpu)
{
    int color;

    PULL_ESTK(color);
    printf("\x1b[%dm", (color & 7) + 40);
    PUSH_ESTK(0);
}
void sysgrplot(M6502 *mpu)
{
    int x, y;

    PULL_ESTK(y);
    PULL_ESTK(x);
    printf("\x1b[%d;%df", y + 1, x * 2 + 1);
    puts("  ");
    PUSH_ESTK(0);
}
void systone(M6502 *mpu)
{
    int pitch, duration;

    PULL_ESTK(duration);
    PULL_ESTK(pitch);
    fprintf(stderr, "CONIO:TONE unimplemented!\r\n");
    PUSH_ESTK(0);
}
void sysrnd(M6502 *mpu)
{
    fprintf(stderr, "CONIO:RND unimplemented!\r\n");
    PUSH_ESTK(rand());
}
/*
 * File I/O
 */
void sysgetpfx(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:GETPFX unimplemented!\r\n");
}
void syssetpfx(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:SETPFX unimplemented!\r\n");
}
void sysgetfileinfo(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:GETFILEINFO unimplemented!\r\n");
}
void syssetfileinfo(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:SETFILEINFO unimplemented!\r\n");
}
void sysgeteof(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:GETEOF unimplemented!\r\n");
}
void sysseteof(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:SETEOF unimplemented!\r\n");
}
void sysiobufs(M6502 *mpu)
{
    uword dummy;
    PULL_ESTK(dummy);
    PUSH_ESTK(0);
    if (trace) printf("IOBUFS\r\n");
}
void sysopen(M6502 *mpu)
{
    uword filestr;
    char filename[128];
    int fd;

    //
    // Pop filename string pointer off stack, make C string and open it
    //
    PULL_ESTK(filestr);
    memcpy(filename, mem_6502 + filestr + 1, mem_6502[filestr]);
    filename[mem_6502[filestr]] = '\0';
    fd = open(filename, O_RDWR);
    if (trace) printf("FILEIO:OPEN(%s): %d\r\n", filename, fd);
    if (fd > 255) fprintf(stderr, "FILEIO:OPEN fd out of range!\r\n");
    if (fd < 0){ fd = 0; *perr = errno;}
    PUSH_ESTK(fd);
}
void sysclose(M6502 *mpu)
{
    int fd, result;

    PULL_ESTK(fd);
    result = close(fd);
    if (result < 0) *perr = errno;
    PUSH_ESTK(result);
    for (result = 0; result < 4; result++)
    {
        if (nlfd[result] == fd)
            nlfd[result] = 0;
    }
}
void sysread(M6502 *mpu)
{
    int fd, len, i, rdlen;
    char rdch;
    uword buf;

    PULL_ESTK(len);
    PULL_ESTK(buf);
    PULL_ESTK(fd);
    if (trace) printf("FILEIO:READ %d $%04X %d\r\n", fd, buf, len);
    for (i = 0; i < 4; i++)
    {
        if (nlfd[i] == fd)
        {
            rdlen = 0;
            while (rdlen < len && read(fd, &rdch, 1))
            {
                if (rdch == '\n') rdch = 0x0D; // Map newline chars
                mem_6502[buf + rdlen++] = rdch;
                if ((rdch & nlmask[i]) == nlchar[i])
                    break;
            }
            PUSH_ESTK(rdlen);
            return;
        }
    }
    len = read(fd, mem_6502 + buf, len);
    if (len < 0) *perr = errno;
    PUSH_ESTK(len);
}
void syswrite(M6502 *mpu)
{
    int fd, len;
    uword buf;

    PULL_ESTK(len);
    PULL_ESTK(buf);
    PULL_ESTK(fd);
    if (trace) printf("FILEIO:WRITE %d $%04X %d\r\n", fd, buf, len);
    len = write(fd, mem_6502 + buf, len);
    if (len < 0) *perr = errno;
    PUSH_ESTK(len);
}
void sysgetmark(M6502 *mpu)
{
    int fd;
    off_t pos;

    PULL_ESTK(fd);
    pos = lseek(fd, 0, SEEK_CUR);
    if (trace) printf("FILEIO:GETMARK %d %lld\r\n", fd, pos);
    if (pos < 0) *perr = errno;
    PUSH_ESTK(pos & 0xFFFF);
    PUSH_ESTK(pos >> 16);
}
void syssetmark(M6502 *mpu)
{
    int fd;
    unsigned int posl, posh;
    off_t pos;

    PULL_ESTK(posh);
    PULL_ESTK(posl);
    PULL_ESTK(fd);
    pos = posl | (posh << 16);
    if (trace) printf("FILEIO:SETMARK %d %lld\r\n", fd, pos);
    pos = lseek(fd, pos, SEEK_SET);
    *perr = 0;
    if (pos < 0) *perr = errno;
    PUSH_ESTK(*perr);
}
void syscreate(M6502 *mpu)
{
    int fd;
    uword filestr, type, aux;
    char filename[128];

    PULL_ESTK(aux);
    PULL_ESTK(type);
    PULL_ESTK(filestr);
    memcpy(filename, mem_6502 + filestr + 1, mem_6502[filestr]);
    filename[mem_6502[filestr]] = '\0';
    /*
    if (type == 0xFE)
        strcat(filename, ".mod");
    */
    fd = creat(filename, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (fd < 0) *perr = errno;
    else close(fd);
    PUSH_ESTK(errno);
}
void sysdestroy(M6502 *mpu)
{
    uword filestr;
    char filename[128];

    PULL_ESTK(filestr);
    memcpy(filename, mem_6502 + filestr + 1, mem_6502[filestr]);
    filename[mem_6502[filestr]] = '\0';
    unlink(filename);
    PUSH_ESTK(errno);
}
void sysrename(M6502 *mpu)
{
    uword newstr, filestr;
    char newname[128], filename[128];

    PULL_ESTK(newstr);
    PULL_ESTK(filestr);
    memcpy(newname, mem_6502 + newstr + 1, mem_6502[newstr]);
    newname[mem_6502[newstr]] = '\0';
    memcpy(filename, mem_6502 + filestr + 1, mem_6502[filestr]);
    filename[mem_6502[filestr]] = '\0';
    rename(filename, newname);
    PUSH_ESTK(errno);
}
void sysnewline(M6502 *mpu)
{
    int fd, mask, nl, i;

    PULL_ESTK(nl);
    PULL_ESTK(mask);
    PULL_ESTK(fd);
    PUSH_ESTK(0);
    for (i = 0; i < 4; i++)
    {
        if (!nlfd[i])
        {
            nlfd[i]   = fd;
            nlmask[i] = mask;
            nlchar[i] = nl;
            break;
        }
    }
}
void sysonline(M6502 *mpu)
{
    uword unit, buf;

    PULL_ESTK(buf);
    PULL_ESTK(unit);
    PUSH_ESTK(0);
    fprintf(stderr, "FILEIO:ONLINE unimplemented!\r\n");
}
void sysunimpl(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:??? unimplemented!\r\n");
}
/*
 * Create PLVM versions of fileio and conio modules.
 */
void export_sysio(void)
{
    byte dci[16];
    uword defaddr;
    uword fileio = alloc_heap(40);
    uword conio  = alloc_heap(26);
    //
    // CMDSYS IO functions
    //
    export_natv("PUTC",  sysputc);
    export_natv("PUTS",  sysputs);
    export_natv("PUTLN", sysputln);
    export_natv("PUTI",  sysputi);
    export_natv("PUTB",  sysputb);
    export_natv("PUTH",  sysputh);
    export_natv("GETS",  sysgets);
    //
    // Exported CONIO function table.
    //
    stodci("CONIO", dci); add_sym(dci, conio);
    defaddr = add_natv(syskeypressed);
    mem_6502[conio + 0] = (byte)defaddr;
    mem_6502[conio + 1] = (byte)(defaddr >> 8);
    defaddr = export_natv("GETC",  sysgetc);
    mem_6502[conio + 2] = (byte)defaddr;
    mem_6502[conio + 3] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysecho);
    mem_6502[conio + 4] = (byte)defaddr;
    mem_6502[conio + 5] = (byte)(defaddr >> 8);
    defaddr = add_natv(syshome);
    mem_6502[conio + 6] = (byte)defaddr;
    mem_6502[conio + 7] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysgotoxy);
    mem_6502[conio + 8] = (byte)defaddr;
    mem_6502[conio + 9] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysviewport);
    mem_6502[conio + 10] = (byte)defaddr;
    mem_6502[conio + 11] = (byte)(defaddr >> 8);
    defaddr = add_natv(systexttype);
    mem_6502[conio + 12] = (byte)defaddr;
    mem_6502[conio + 13] = (byte)(defaddr >> 8);
    defaddr = add_natv(systextmode);
    mem_6502[conio + 14] = (byte)defaddr;
    mem_6502[conio + 15] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysgrmode);
    mem_6502[conio + 16] = (byte)defaddr;
    mem_6502[conio + 17] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysgrcolor);
    mem_6502[conio + 18] = (byte)defaddr;
    mem_6502[conio + 19] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysgrplot);
    mem_6502[conio + 20] = (byte)defaddr;
    mem_6502[conio + 21] = (byte)(defaddr >> 8);
    defaddr = add_natv(systone);
    mem_6502[conio + 22] = (byte)defaddr;
    mem_6502[conio + 23] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysrnd);
    mem_6502[conio + 24] = (byte)defaddr;
    mem_6502[conio + 25] = (byte)(defaddr >> 8);
    //
    // Exported FILEIO function table.
    //
    stodci("FILEIO", dci); add_sym(dci, fileio);
    defaddr = add_natv(sysgetpfx);
    mem_6502[fileio + 0] = (byte)defaddr;
    mem_6502[fileio + 1] = (byte)(defaddr >> 8);
    defaddr = add_natv(syssetpfx);
    mem_6502[fileio + 2] = (byte)defaddr;
    mem_6502[fileio + 3] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysgetfileinfo);
    mem_6502[fileio + 4] = (byte)defaddr;
    mem_6502[fileio + 5] = (byte)(defaddr >> 8);
    defaddr = add_natv(syssetfileinfo);
    mem_6502[fileio + 6] = (byte)defaddr;
    mem_6502[fileio + 7] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysgeteof);
    mem_6502[fileio + 8] = (byte)defaddr;
    mem_6502[fileio + 9] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysseteof);
    mem_6502[fileio + 10] = (byte)defaddr;
    mem_6502[fileio + 11] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysiobufs);
    mem_6502[fileio + 12] = (byte)defaddr;
    mem_6502[fileio + 13] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysopen); // sysopen
    mem_6502[cmdsys + 8] =  (byte)defaddr;
    mem_6502[cmdsys + 9] =  (byte)(defaddr >> 8);
    mem_6502[fileio + 14] = (byte)defaddr;
    mem_6502[fileio + 15] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysclose); // sysclose
    mem_6502[cmdsys + 10] = (byte)defaddr;
    mem_6502[cmdsys + 11] = (byte)(defaddr >> 8);
    mem_6502[fileio + 16] = (byte)defaddr;
    mem_6502[fileio + 17] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysread); // sysread
    mem_6502[cmdsys + 12] = (byte)defaddr;
    mem_6502[cmdsys + 13] = (byte)(defaddr >> 8);
    mem_6502[fileio + 18] = (byte)defaddr;
    mem_6502[fileio + 19] = (byte)(defaddr >> 8);
    defaddr = add_natv(syswrite); // syswrite
    mem_6502[cmdsys + 14] = (byte)defaddr;
    mem_6502[cmdsys + 15] = (byte)(defaddr >> 8);
    mem_6502[fileio + 20] = (byte)defaddr;
    mem_6502[fileio + 21] = (byte)(defaddr >> 8);
    defaddr = add_natv(syscreate);
    mem_6502[fileio + 22] = (byte)defaddr;
    mem_6502[fileio + 23] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysdestroy);
    mem_6502[fileio + 24] = (byte)defaddr;
    mem_6502[fileio + 25] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysrename);
    mem_6502[fileio + 26] = (byte)defaddr;
    mem_6502[fileio + 27] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysnewline);
    mem_6502[fileio + 28] = (byte)defaddr;
    mem_6502[fileio + 29] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysonline);
    mem_6502[fileio + 30] = (byte)defaddr;
    mem_6502[fileio + 31] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysunimpl); // readblock
    mem_6502[fileio + 32] = (byte)defaddr;
    mem_6502[fileio + 33] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysunimpl); // writeblock
    mem_6502[fileio + 34] = (byte)defaddr;
    mem_6502[fileio + 35] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysgetmark); // getmark
    mem_6502[fileio + 36] = (byte)defaddr;
    mem_6502[fileio + 37] = (byte)(defaddr >> 8);
    defaddr = add_natv(syssetmark); // setmark
    mem_6502[fileio + 38] = (byte)defaddr;
    mem_6502[fileio + 39] = (byte)(defaddr >> 8);
}
