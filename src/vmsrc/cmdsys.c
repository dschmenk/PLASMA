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
 * Load module defines
 */
#define REL_ADDR            0x1000 // Address modules are compiled at to be subtracted off
#define SYSFLAG_KEEP        0x2000
#define SYSFLAG_INITKEEP    0x4000
/*
 * Symbol table
 */
#define SYMTBL_SIZE         1024
#define SYM_LEN             16
#define DEF_ENTRY_SIZE      5
byte  symtbl[SYMTBL_SIZE];     // Symbol table outside mem_6502 for PLVM
byte *lastsym  = symtbl;
uword keybdbuf = 0x0200;
uword heap     = 0x0300;
/*
 * Standard Library exported functions.
 */

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
int setPLVMTraps(M6502 *mpu)
{
    //
    // Hooks for BRK, enter VM or native code
    //
    M6502_setCallback(mpu, call, VM_INLINE_ENTRY, vm_indef);
    M6502_setCallback(mpu, call, VM_EXT_ENTRY,    vm_exdef);
    M6502_setCallback(mpu, call, VM_NATV_ENTRY,   vm_natvdef);
    M6502_setCallback(mpu, call, 0x0000, vm_irq);
    return 0;
}
int setApple1Traps(M6502 *mpu)
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
    // !!! Hook Apple 1 VM entrypoints for testing purposes !!!
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
uword alloc_heap(uword size)
{
    uword vm_fp = UWORD_PTR(&mem_6502[FP]);
    uword addr = heap;
    heap += size;
    if (heap >= vm_fp)
        pfail("heap/frame collision.");
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
 * Symbol table routines.
 */
void dump_sym(void)
{
    int len;
    byte *tbl;

    tbl = symtbl;
    printf("\nSystem Symbol Table:\n");
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
uword lookup_sym(byte *dci)
{
    char str[20];
    byte *match, *entry = symtbl;
    while (*entry)
    {
        match = dci;
        while (*entry == *match)
        {
            if (!(*entry++ & 0x80)) // End of DCI string
                return *(uword *)entry;
            match++;
        }
        while (*entry++ & 0x80); // Skip to next entry
        entry += 2;
    }
    dcitos(dci, str);
    fprintf(stderr, "\nSymbol %s ", str);
    pfail("not found in symbol table");
    return 0;
}
int add_sym(byte *dci, uword addr)
{
    while (*dci & 0x80)
        *lastsym++ = *dci++;
    *lastsym++ = *dci;
    *(uword *)lastsym = addr;
    lastsym += 2;
    if (lastsym >= &symtbl[SYMTBL_SIZE])
        pfail("Symbol table overflow");
    return 0;
}
/*
 * DEF routines - Create entryoint for 6502 that calls out to PLVM or native code
 */
byte *add_def(byte type, uword haddr, byte *lastdef)
{
    *lastdef++ = 0x20; // JSR opcode
    switch (type)
    {
        case VM_NATV_DEF:
            *lastdef++ = VM_NATV_ENTRY & 0xFF;
            *lastdef++ = VM_NATV_ENTRY >> 8;
            break;
        case VM_EXT_DEF:
            *lastdef++ = VM_EXT_ENTRY & 0xFF;
            *lastdef++ = VM_EXT_ENTRY >> 8;
            break;
        case VM_INLINE_DEF: // Never happed
            *lastdef++ = VM_INLINE_ENTRY & 0xFF;
            *lastdef++ = VM_INLINE_ENTRY >> 8;
        default:
            pfail("Add unknown DEF type");
    }
    //
    // Follow with memory handle
    //
    *lastdef++ = haddr;
    *lastdef++ = haddr >> 8;
    return lastdef;
}
uword lookup_def(byte *deftbl, int defaddr)
{
    int calldef = 0;
    while (*deftbl)
    {
        if ((deftbl[3] | (deftbl[4] << 8)) == defaddr)
        {
            calldef = deftbl - mem_6502;
            break;
        }
        deftbl += 5;
    }
    return (uword)calldef;
}
/*
 * Relocation routines.
 */
uword lookup_extern(byte *esd, int index)
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
    fprintf(stderr, "Extern index %d", index);
    pfail(" not found in ESD");
    return 0;
}
/*
 * Load module routine - most compilcated routine in all PLVM. Sorry
 */
int load_mod(M6502 *mpu, byte *mod)
{
    uword modsize, hdrlen, len, end, magic, bytecode, fixup, addr, modaddr;
    uword sysflags = 0, defcnt = 0, init = 0;
    word modfix, modofst;
    byte *deftbl, *deflast, *moddep, *rld, *esd, *sym;
    byte header[128];
    int fd;
    char filename[48], string[17];

    dcitos(mod, filename);
    printf("Load module %s\n", filename);
    if (strlen(filename) < 4 || (filename[strlen(filename) - 4] != '.'))
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
                //
                // Re-read first 128 bytes of module
                //
                fd = open(filename, O_RDONLY, 0);
                len = read(fd, header, 128);
            }
        }
        //
        // Allocate heap space for DEF table.
        //
        deftbl   = &mem_6502[alloc_heap(defcnt * 5 + 1)];
        deflast  = deftbl;
        *deflast = 0;
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
        modfix    = modaddr  - hdrlen + 2;
        modofst   = modfix - REL_ADDR;
        bytecode += modofst;
        end       = modfix + modsize;
        rld       = mem_6502 + end;       // Re-Locatable Directory
        esd       = rld;                  // Extern+Entry Symbol Directory
        for (esd = rld; *esd; esd += 4);  // Scan to end of RLD
        esd++;                            // ESD starts just past RLD
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
            printf("Module init: $%04X\n", init ? init + modofst : 0);
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
        while (rld[0])
        {
            addr = rld[1] | (rld[2] << 8);
            if (rld[0] == 0x02)
            {
                if (show_state) printf("\tDEF               CODE");
                //
                // This is a bytcode def entry - add it to the def directory.
                //
                addr   += modofst;
                deflast = add_def(VM_EXT_DEF, addr, deflast);
            }
            else
            {
                addr += modfix;
                if (rld[0] & 0x80)
                    //
                    // 16 bit fixup
                    //
                    fixup = (mem_6502[addr] | (mem_6502[addr + 1] << 8));
                else
                {
                    //
                    // 8 bit fixups
                    //
                    if (rld[0] & 0x40)
                        fixup = rld[3] + (mem_6502[addr] << 8);
                    else
                        fixup = mem_6502[addr];
                }
                if (rld[0] & 0x10)
                {
                    //
                    // Resolve external symbol
                    //
                    if (show_state) printf("\tEXTERN[$%02X]       ", rld[3]);
                    fixup += lookup_extern(esd, rld[3]);
                }
                else
                {
                    //
                    // Internal symbol
                    //
                    fixup += modofst;
                    if (fixup >= bytecode)
                    {
                        //
                        // Replace with call def dictionary.
                        //
                        if (show_state) printf("\tDEF[$%04X->", fixup);
                        fixup = lookup_def(deftbl, fixup);
                        if (show_state) printf("$%04X] ", fixup);
                    }
                    else
                        if (show_state) printf("\tINTERN            ");
                }
                if (rld[0] & 0x80)
                {
                    //
                    // 16 bit fixup
                    //
                    if (show_state) printf("WORD");
                    mem_6502[addr]     = fixup;
                    mem_6502[addr + 1] = fixup >> 8;
                }
                else
                {
                    if (show_state) printf(rld[0] & 0x40 ? "MSBYTE" : "LSBYTE");
                    if (rld[0] & 0x40)
                        //
                        // 8 bit MSB fixup
                        //
                        mem_6502[addr] = fixup >> 8;
                    else
                        //
                        // 8 bit LSB fixup
                        //
                        mem_6502[addr] = fixup;
                }
            }
            if (show_state) printf("@$%04X\n", addr);
            rld += 4;
        }
        if (show_state) printf("\nExternal/Entry Symbol Directory:\n");
        while (esd[0])
        {
            sym = esd;
            esd += dcitos(esd, string);
            if (esd[0] & 0x08)
            {
                //
                // Export symbol
                //
                addr = esd[1] | (esd[2] << 8);
                addr += modofst;
                if (show_state) printf("\tEXPORT %s@$%04X\n", string, addr);
                if (addr >= bytecode)
                    //
                    // Convert to def entry address
                    //
                    addr = lookup_def(deftbl, addr);
                //
                // Add symbol to PLASMA symbol table
                //
                add_sym(sym, addr);
            }
            esd += 3;
        }
    }
    else
    {
        fprintf(stderr, "Unable to load module %s", filename);
        pfail("");
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
        vm_interp(mpu, mem_6502 + init + modfix - REL_ADDR);
        if (!(sysflags & SYSFLAG_INITKEEP))
            release_heap(init + modofst); // Free up init code
        init  = mem_6502[++mpu->registers->s + 0x100];
        init |= mem_6502[++mpu->registers->s + 0x100] << 8;
        return init;
    }
    return 0;
}
/*
 * Native CMDSYS routines
 */
void sysputs(M6502 *mpu)
{
    uword strptr;
    char cstr[256];
    //
    // Pop string pointer off stack, copy into C string, and puts it
    //
    strptr  = mem_6502[ESTKL + mpu->registers->x];
    strptr |= mem_6502[ESTKH + mpu->registers->x] << 8;
    memcpy(cstr, mem_6502 + strptr + 1, mem_6502[strptr]);
    cstr[mem_6502[strptr]] = '\0';
    puts(cstr);
}
void sysputln(M6502 *mpu)
{
    puts("\n");
}
/*
 * CMDSYS exports
 */
void export_cmdsys(void)
{
    byte dci[16];
    uword haddr, defaddr, cmdsys = alloc_heap(23);
    byte  *lastdef = mem_6502 + alloc_heap(26*2);
    *lastdef     = 0;
    stodci("CMDSYS", dci); add_sym(dci, cmdsys);
    haddr   = vm_addnatv(sysputs);
    defaddr = (uword)(lastdef - mem_6502);
    lastdef = add_def(VM_NATV_DEF, haddr, lastdef);
    stodci("PUTS", dci); add_sym(dci, defaddr);
    haddr   = vm_addnatv(sysputln);
    defaddr = (uword)(lastdef - mem_6502);
    lastdef = add_def(VM_NATV_DEF, haddr, lastdef);
    stodci("PUTLN", dci); add_sym(dci, defaddr);
#if 0
word version      = 0x0211; // 02.11
word syspath;
word cmdlnptr;
word              = sysexecmod, sysopen, sysclose, sysread, syswrite;
byte perr;
byte jitcount     = 0;
byte jitsize      = 0;
byte refcons      = 0;
byte devcons      = 0;
word              = syslookuptbl

char machidstr[]  = "MACHID";
char sysstr[]     = "SYSCALL";
char callstr[]    = "CALL";
char putcstr[]    = "PUTC";
char putlnstr[]   = "PUTLN";
char putsstr[]    = "PUTS";
char putistr[]    = "PUTI";
char putbstr[]    = "PUTB";
char putwstr[]    = "PUTH";
char getcstr[]    = "GETC";
char getsstr[]    = "GETS";
char toupstr[]    = "TOUPPER";
char strcpystr[]  = "STRCPY";
char strcatstr[]  = "STRCAT";
char hpmarkstr[]  = "HEAPMARK";
char hpalignstr[] = "HEAPALLOCALIGN";
char hpallocstr[] = "HEAPALLOC";
char hprelstr[]   = "HEAPRELEASE";
char hpavlstr[]   = "HEAPAVAIL";
char memsetstr[]  = "MEMSET";
char memcpystr[]  = "MEMCPY";
char uisgtstr[]   = "ISUGT";
char uisgestr[]   = "ISUGE";
char uisltstr[]   = "ISULT";
char uislestr[]   = "ISULE";
char sextstr[]    = "SEXT";
char divmodstr[]  = "DIVMOD";
char

word exports[]    = @sysmodstr, @version
word              = @sysstr,    @syscall
word              = @callstr,   @call
word              = @putcstr,   @cout
word              = @putlnstr,  @crout
word              = @putsstr,   @prstr
word              = @putistr,   @print
word              = @putbstr,   @prbyte
word              = @putwstr,   @prword
word              = @getcstr,   @cin
word              = @getsstr,   @rdstr
word              = @toupstr,   @toupper
word              = @hpmarkstr, @markheap
word              = @hpallocstr,@allocheap
word              = @hpalignstr,@allocalignheap
word              = @hprelstr,  @releaseheap
word              = @hpavlstr,  @availheap
word              = @memsetstr, @memset
word              = @memcpystr, @memcpy
word              = @strcpystr, @strcpy
word              = @strcatstr, @strcat
word              = @uisgtstr,  @uword_isgt
word              = @uisgestr,  @uword_isge
word              = @uisltstr,  @uword_islt
word              = @uislestr,  @uword_isle
word              = @sextstr,   @sext
word              = @divmodstr, @divmod
word              = @machidstr, @machid
word              = 0
#endif
}
int main(int argc, char **argv)
{
    byte   dci[32];
    int    i;
    char  *cmdfile    = "a1plasma.bin";
    char  *modfile = NULL;
    //
    // Init 6502 emulator/VM
    //
    M6502 *mpu = M6502_new(0, mem_6502, 0);
    M6502_reset(mpu);
    mpu->registers->x = ESTK_SIZE;
    //
    // Parse command line options
    //
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
            modfile = *argv;
    }
    //
    // Run PLVM or Apple1 environment based on passed in modfile
    //
    if (modfile)
    {
        setPLVMTraps(mpu);
        //
        // 64K - 256 RAM
        //
        mem_6502[FPL] = 0x00;
        mem_6502[FPH] = 0xFF;
        mem_6502[PPL] = 0x00;
        mem_6502[PPH] = 0xFF;
        //
        // Load module from command line - PLVM version
        //
        export_cmdsys();
        stodci(modfile, dci);
        if (show_state) dump_sym();
        load_mod(mpu, dci);
        if (show_state) dump_sym();
    }
    else
    {
        setApple1Traps(mpu);
        //
        // 32K RAM
        //
        mem_6502[FPL] = 0x00;
        mem_6502[FPH] = 0x80;
        mem_6502[PPL] = 0x00;
        mem_6502[PPH] = 0x80;
        //
        // Load Apple 1 emulation - lib6502 version
        //
        if (!load(0x280, cmdfile))
            pfail(cmdfile);
        setRawInput();
        mpu->registers->pc = 0x0280;
        M6502_exec(mpu);
    }
    M6502_delete(mpu);
    return 0;
}
