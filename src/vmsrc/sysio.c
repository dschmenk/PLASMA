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
#include <sys/filio.h>
#include <termios.h>
#include "plvm.h"

int keyqueue = 0;

/*
 * Console I/O
 */
void sysputc(M6502 *mpu)
{
    char c;
    //
    // Pop char and putchar it
    //
    PULL_ESTK(c);
    putchar(c);
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
                putchar('\n');
                break;
            default:
                putchar(ch);
        }
    }
}
void sysputln(M6502 *mpu)
{
    putchar('\n');
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
    int n;

    if (ioctl(STDIN_FILENO, FIONREAD, &n) == 0 && n > 0)
        keyqueue = getchar() | 0x80;
    PUSH_ESTK(keyqueue);
}
void sysgetc(M6502 *mpu)
{
    char c;
    //
    // Push getchar()
    //
    if (keyqueue)
    {
        c = keyqueue & 0x7F;
        keyqueue = 0;
    }
    else
        c = getchar();
    PUSH_ESTK(c);
}
void sysgets(M6502 *mpu)
{
    uword strptr;
    int len;
    char instr[256];

    //
    // Push gets(), limiting it to 128 chars
    //
    PULL_ESTK(instr[0]);
    putchar(instr[0] & 0x7F);
    len = strlen(gets(instr));
    if (len > 128)
        len = 128;
    mem_6502[CMDLINE_STR] = len;
    memcpy(mem_6502 + CMDLINE_BUF, instr, len);
    PUSH_ESTK(CMDLINE_STR);
}
void sysecho(M6502 *mpu)
{
    fprintf(stderr, "CONIO:ECHO unimplemented!\n");
}
void syshome(M6502 *mpu)
{
    fprintf(stderr, "CONIO:HOME unimplemented!\n");
}
void sysgotoxy(M6502 *mpu)
{
    fprintf(stderr, "CONIO:GOTOXY unimplemented!\n");
}
void sysviewport(M6502 *mpu)
{
    fprintf(stderr, "CONIO:VIEWPORT unimplemented!\n");
}
void systexttype(M6502 *mpu)
{
    fprintf(stderr, "CONIO:TEXTYPE unimplemented!\n");
}
void systextmode(M6502 *mpu)
{
    fprintf(stderr, "CONIO:TEXTMODE unimplemented!\n");
}
void sysgrmode(M6502 *mpu)
{
    fprintf(stderr, "CONIO:GRMODE unimplemented!\n");
}
void sysgrcolor(M6502 *mpu)
{
    fprintf(stderr, "CONIO:GRCOLOR unimplemented!\n");
}
void sysgrplot(M6502 *mpu)
{
    fprintf(stderr, "CONIO:GRPLOT unimplemented!\n");
}
void systone(M6502 *mpu)
{
    fprintf(stderr, "CONIO:TONE unimplemented!\n");
}
void sysrnd(M6502 *mpu)
{
    fprintf(stderr, "CONIO:RND unimplemented!\n");
}
/*
 * File I/O
 */
void sysgetpfx(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:GETPFX unimplemented!\n");
}
void syssetpfx(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:SETPFX unimplemented!\n");
}
void sysgetfileinfo(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:GETFILEINFO unimplemented!\n");
}
void syssetfileinfo(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:SETFILEINFO unimplemented!\n");
}
void sysgeteof(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:GETEOF unimplemented!\n");
}
void sysseteof(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:SETEOF unimplemented!\n");
}
void sysiobufs(M6502 *mpu)
{
    uword dummy;
    PULL_ESTK(dummy);
    PUSH_ESTK(0);
    if (trace) printf("IOBUFS\n");
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
    if (trace) printf("FILEIO:OPEN(%s)\n", filename);
    fd = open(filename, O_RDWR);
    if (fd > 255) fprintf(stderr, "FILEIO:OPEN fd out of range!\n");
    if (fd < 0) *perr = errno;
    PUSH_ESTK(fd);
}
void sysclose(M6502 *mpu)
{
    int fd, result;

    PULL_ESTK(fd);
    result = close(fd);
    if (result < 0) *perr = errno;
    PUSH_ESTK(result);
}
void sysread(M6502 *mpu)
{
    int fd, len;
    uword buf;

    PULL_ESTK(len);
    PULL_ESTK(buf);
    PULL_ESTK(fd);
    len = read(fd, mem_6502 + buf, len);
    if (len < 0) *perr = errno;
    PUSH_ESTK(len);
}
void syswrite(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:WRITE unimplemented!\n");
}
void syscreate(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:CREATE unimplemented!\n");
}
void sysdestroy(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:DESTROY unimplemented!\n");
}
void sysrename(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:RENAME unimplemented!\n");
}
void sysnewline(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:NEWLINE unimplemented!\n");
}
void sysonline(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:ONLINE unimplemented!\n");
}
void sysunimpl(M6502 *mpu)
{
    fprintf(stderr, "FILEIO:??? unimplemented!\n");
}
/*
 * Create PLVM versions of fileio and conio modules.
 */
void export_sysio(void)
{
    byte dci[16];
    uword defaddr;
    uword fileio = alloc_heap(36);
    uword conio  = alloc_heap(24);
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
//word conio[]
//word = @a2keypressed
//word = @getc
//word = @a12echo
//word = @a2home
//word = @a2gotoxy
//word = @a2viewport
//word = @a2texttype
//word = @a2textmode
//word = @a2grmode
//word = @a2grcolor
//word = @a2grplot
//word = @a2tone
//word = @a2rnd
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
    defaddr = add_natv(sysgrplot);
    mem_6502[conio + 18] = (byte)defaddr;
    mem_6502[conio + 19] = (byte)(defaddr >> 8);
    defaddr = add_natv(systone);
    mem_6502[conio + 20] = (byte)defaddr;
    mem_6502[conio + 21] = (byte)(defaddr >> 8);
    defaddr = add_natv(sysrnd);
    mem_6502[conio + 22] = (byte)defaddr;
    mem_6502[conio + 23] = (byte)(defaddr >> 8);
    //
    // Exported FILEIO function table.
    //
//word fileio[]
//word = @a2getpfx, @a2setpfx, @a2getfileinfo, @a2setfileinfo, @a23geteof, @a23seteof, @a2iobufs, @a2open, @a2close
//word = @a23read, @a2write, @a2create, @a23destroy, @a23rename
//word = @a2newline, @a2online, @a2readblock, @a2writeblock
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
}
