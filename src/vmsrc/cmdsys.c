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
byte *perr;
int paused = 0;
byte keyqueue = 0;
uword keybdbuf = 0x0200;
uword heap     = 0x0300;
uword cmdsys;
typedef struct {
    uword modofst;
    uword defaddr;
} defxlat_t;
/*
 * Hard fail
 */
void pfail(const char *msg)
{
    fflush(stdout);
    tcsetattr(STDIN_FILENO, TCSANOW, &org_tio);
    perror(msg);
    exit(1);
}
/*
 * M6502_run() with trace/single step ability
 */
byte keypressed(M6502 *mpu)
{
    if (!(keyqueue & 0x80))
    {
        if (read(STDIN_FILENO, &keyqueue, 1) > 0)
            keyqueue |= 0x80;
    }
    return keyqueue;
}
byte keyin(M6502 *mpu)
{
    byte cin = keyqueue;
    keyqueue = 0;
    while (!cin)
    {
        if (read(STDIN_FILENO, &cin, 1) > 0)
        {
            if (cin == 0x03) // CTRL-C
            {
                cin = 0;
                exit(-1);
            }
        }
        else
            usleep(10000);
    }
    return cin & 0x7F;
}
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
            if ((trace == SINGLE_STEP) && (paused || (keypressed(mpu) && keyqueue == 0x83)))
            {
                keyqueue = 0;
                while (!keypressed(mpu));
                if (keyqueue == (0x80|'C'))
                    paused = 0;
                else if (keyqueue == (0x80|'Q'))
                    break;
                else if (keyqueue == 0x9B) // Escape (QUIT)
                    exit(-1);
                keyqueue = 0;
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
    termio.c_iflag     = BRKINT | IGNPAR;
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
    printf("\r\nSystem Symbol Table:\r\n");
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
        printf("$%04X\r\n", tbl[0] | (tbl[1] << 8));
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
    if (trace) printf("\r\nSymbol %s not found in symbol table\r\n", str);
    return 0;
}
uword add_sym(byte *dci, uword addr)
{
    while (*dci & 0x80)
        *lastsym++ = *dci++;
    *lastsym++ = *dci;
    *(uword *)lastsym = addr;
    lastsym += 2;
    if (lastsym >= &symtbl[SYMTBL_SIZE])
        pfail("Symbol table overflow");
    return addr;
}
/*
 * DEF routines - Create entryoint for 6502 that calls out to PLVM or native code
 */
void dump_def(defxlat_t *defxtbl, uword defcnt)
{
    if (defcnt)
    {
        printf("DEF XLATE table:\r\n");
        while (defcnt--)
        {
            printf("$%04X -> $%04X\r\n", defxtbl->modofst, defxtbl->defaddr);
            defxtbl++;
        }
    }
}
byte *add_def(byte type, uword haddr, byte *lastdef)
{
    *lastdef++ = 0x20; // JSR opcode
    switch (type)
    {
        case VM_NATV_DEF:
            *lastdef++ = (byte)VM_NATV_ENTRY;
            *lastdef++ = (byte)(VM_NATV_ENTRY >> 8);
            break;
        case VM_EXT_DEF:
            *lastdef++ = (byte)VM_EXT_ENTRY;
            *lastdef++ = (byte)(VM_EXT_ENTRY >> 8);
            break;
        case VM_DEF:
            *lastdef++ = (byte)VM_INDIRECT_ENTRY;
            *lastdef++ = (byte)(VM_INDIRECT_ENTRY >> 8);
            break;
        case VM_INLINE_DEF: // Never happen
            *lastdef++ = (byte)VM_INLINE_ENTRY;
            *lastdef++ = (byte)(VM_INLINE_ENTRY >> 8);
        default:
            pfail("Add unknown DEF type");
    }
    //
    // Follow with memory handle
    //
    *lastdef++ = (byte)haddr;
    *lastdef++ = (byte)(haddr >> 8);
    return lastdef;
}
void xlat_def(uword addr, uword ofst, defxlat_t *defxtbl)
{
    while (defxtbl->defaddr)
        defxtbl++;
    defxtbl->modofst = ofst;
    defxtbl->defaddr = addr;
}
uword lookup_def(defxlat_t *defxtbl, uword ofst)
{
    while (defxtbl->defaddr && defxtbl->modofst != ofst)
        defxtbl++;
    return defxtbl->defaddr;
}
uword add_natv(VM_Callout natvfn)
{
    uword handle, defaddr;
    handle  = vm_addnatv(natvfn);
    defaddr = alloc_heap(5);
    add_def(VM_NATV_DEF, handle, mem_6502 + defaddr);
    return defaddr;
}
uword export_natv(char *symstr, VM_Callout natvfn)
{
    byte dci[16];
    stodci(symstr, dci);
    return add_sym(dci, add_natv(natvfn));
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
    code *extcode;
    defxlat_t *defxtbl;
    byte header[128];
    int fd;
    char filename[128], string[17];

    dcitos(mod, filename);
    /*
    if (strlen(filename) < 4 || (filename[strlen(filename) - 4] != '.'))
        strcat(filename, ".mod");
    */
    fd = open(filename, O_RDONLY, 0);
    if (fd <= 0 && filename[0] != '/' && strlen(filename) < 17)
    {
        //
        // Search syspath if module not found
        //
        strcpy(string, filename);
        strcpy(filename, (char *)mem_6502 + SYSPATH_BUF);
        strcat(filename, string);
        fd = open(filename, O_RDONLY, 0);
    }
    if (trace) printf("Load module %s: %d\r\n", filename, fd);
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
        else
        {
            //
            // Standard REL file - probaby generated from EDASM
            //
            sysflags = 0;
            bytecode = 0;
            defcnt   = 0;
            init     = 0;
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
        if (trace)
        {
            //
            // Dump different parts of module.
            //
            printf("Module load addr: $%04X\r\n", modaddr);
            printf("Module size: $%04X (%d)\r\n", modsize, modsize);
            printf("Module code+data size: $%04X (%d)\r\n", end - modaddr, end - modaddr);
            printf("Module magic: $%04X\r\n", magic);
            printf("Module sysflags: $%04X\r\n", sysflags);
            if (defcnt)
            {
                printf("Module def count: %d\r\n", defcnt);
                printf("Module bytecode: $%04X\r\n", bytecode);
                printf("Module init: $%04X\r\n", init ? init + modofst : 0);
                printf("Module def size: $%04X (%d)\r\n", end - bytecode, end - bytecode);
                printf("Module def end: $%04X\r\n", end);
            }
        }
        //
        // Add module to symbol table.
        //
        add_sym(mod, modaddr);
        //
        // Allocate external code memory
        //
        if (defcnt)
        {
            extcode = code_alloc(end - bytecode);
            defxtbl = malloc(sizeof(defxlat_t) * defcnt);
            memset(defxtbl, 0, sizeof(defxlat_t) * defcnt);
        }
        //
        // Print out the Re-Location Dictionary.
        //
        if (trace)
            printf("\r\nRe-Location Dictionary:\r\n");
        while (rld[0])
        {
            addr = rld[1] | (rld[2] << 8);
            if (rld[0] == 0x02)
            {
                if (trace) printf("\tDEF               CODE");
                //
                // This is a bytcode def entry - add it to the def directory.
                //
                addr += modofst;
                xlat_def(deflast - mem_6502, addr, defxtbl);
                deflast = add_def(VM_EXT_DEF, vm_addxdef(extcode + addr - bytecode), deflast);
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
                    if (trace) printf("\tEXTERN[$%02X]       ", rld[3]);
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
                        if (trace) printf("\tDEF[$%04X->", fixup);
                        fixup = lookup_def(defxtbl, fixup);
                        if (trace) printf("$%04X] ", fixup);
                        if (!fixup) pfail("Unresolved DEF");
                    }
                    else
                        if (trace) printf("\tINTERN            ");
                }
                if (rld[0] & 0x80)
                {
                    //
                    // 16 bit fixup
                    //
                    if (trace) printf("WORD");
                    mem_6502[addr]     = fixup;
                    mem_6502[addr + 1] = fixup >> 8;
                }
                else
                {
                    if (trace) printf(rld[0] & 0x40 ? "MSBYTE" : "LSBYTE");
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
            if (trace) printf("@$%04X\r\n", addr);
            rld += 4;
        }
        if (trace) printf("\r\nExternal/Entry Symbol Directory:\r\n");
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
                if (trace) printf("\tEXPORT %s@$%04X\r\n", string, addr);
                if (addr >= bytecode)
                    //
                    // Convert to def entry address
                    //
                    addr = lookup_def(defxtbl, addr);
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
        fprintf(stderr, "\r\nUnable to load module %s: ", filename);
        pfail("");
    }
    //
    // Reserve heap space for relocated module and copy bytecode extrnally.
    //
    if (defcnt)
    {
        if (init) defcnt--;
        if (trace) dump_def(defxtbl, defcnt);
        free(defxtbl);
        if (trace) printf("Copy bytecode from $%04X to 0x%08X, size %d\r\n", bytecode, (unsigned int)extcode, end - bytecode);
        memcpy(extcode, mem_6502 + bytecode, end - bytecode);
        end = bytecode; // Free up bytecode in main memory
    }
    alloc_heap(end - modaddr);
    //
    //Call init routine.
    //
    if (init)
    {
        vm_interp(mpu, extcode + init + modofst - bytecode);
        //if (!(sysflags & SYSFLAG_INITKEEP))
        //    release_heap(init + modofst); // Free up init code
        PULL_ESTK(init);
        return init;
    }
    return 0;
}
/*
 * Native CMDSYS routines
 */
void sysexecmod(M6502 *mpu)
{
    uword modstr;
    char modfile[128];
    byte dci[128];
    int result;

    PULL_ESTK(modstr);
    memcpy(modfile, mem_6502 + modstr + 1, mem_6502[modstr]);
    modfile[mem_6502[modstr]] = '\0';
    stodci(modfile, dci);
    result = load_mod(mpu, dci);
    PUSH_ESTK(result);
}
void syslookuptbl(M6502 *mpu)
{
    uword sym, addr;
    char symbol[32];

    PULL_ESTK(sym);
    addr = lookup_sym(mem_6502 + sym);
    PUSH_ESTK(addr);
    /*if (trace)*/
    {
        dcitos(mem_6502 + sym, symbol);
        printf("LOOKUPSYM: %s => $%04X\r\n", symbol, addr);
    }
}
void syscall6502(M6502 *mpu)
{
    uword params;
    byte  cmd, status;

    PULL_ESTK(params);
    PULL_ESTK(cmd);
    status = 0;
    switch (cmd)
    {
    }
    fprintf(stderr, "SYSCALL6502 unimplemented!\r\n");
    PUSH_ESTK(status);
}
void systoupper(M6502 *mpu)
{
    char c;
    PULL_ESTK(c);
    c = toupper(c);
    PUSH_ESTK(c);
    if (trace) printf("TOUPPER\r\n");
}
void sysstrcpy(M6502 *mpu)
{
    uword src, dst;
    PULL_ESTK(src);
    PULL_ESTK(dst);
    memcpy(mem_6502 + dst, mem_6502 + src, mem_6502[src] + 1);
    PUSH_ESTK(dst);
    if (trace) printf("STRCPY\r\n");
}
void sysstrcat(M6502 *mpu)
{
    uword src, dst;
    PULL_ESTK(src);
    PULL_ESTK(dst);
    memcpy(mem_6502 + dst + mem_6502[dst] +  1, mem_6502 + src + 1, mem_6502[src]);
    mem_6502[dst] += mem_6502[src];
    PUSH_ESTK(dst);
    if (trace) printf("STRCAT\r\n");
}
void sysmemset(M6502 *mpu)
{
    uword dst, val, size;
    PULL_ESTK(size);
    PULL_ESTK(val);
    PULL_ESTK(dst);
    while (size > 1)
    {
        mem_6502[dst++] = (byte)val;
        mem_6502[dst++] = (byte)(val >> 8);
        size -= 2;
    }
    if (size)
        mem_6502[dst] = (byte)val;
    if (trace) printf("MEMSET\r\n");
}
void sysmemcpy(M6502 *mpu)
{
    uword dst, src, size;
    PULL_ESTK(size);
    PULL_ESTK(src);
    PULL_ESTK(dst);
    memcpy(mem_6502 + dst, mem_6502 + src, size);
    if (trace) printf("MEMCPY\r\n");
}
void sysheapmark(M6502 *mpu)
{
    PUSH_ESTK(heap);
    if (trace) printf("HEAPMARK\r\n");
}
void sysheapallocalign(M6502 *mpu)
{
    uword size, pow2, align, addr, freeaddr;

    PULL_ESTK(freeaddr);
    PULL_ESTK(pow2);
    PULL_ESTK(size);
    align = (1 << pow2) - 1;
    mem_6502[freeaddr]     = (byte)heap;
    mem_6502[freeaddr + 1] = (byte)(heap >> 8);
    addr  = (heap + align) & ~align;
    heap += size;
    PUSH_ESTK(addr);
    if (trace) printf("HEAPALLOCALIGN\r\n");
}
void sysheapalloc(M6502 *mpu)
{
    uword size, addr;

    PULL_ESTK(size);
    addr = alloc_heap(size);
    PUSH_ESTK(addr);
    if (trace) printf("HEAPALLOC\r\n");
}
void sysheaprelease(M6502 *mpu)
{
    uword avail, vm_fp = UWORD_PTR(&mem_6502[FP]);
    PULL_ESTK(heap);
    avail = vm_fp - heap;
    PUSH_ESTK(avail);
    if (trace) printf("HEAPRELEASE\r\n");
}
void sysheapavail(M6502 *mpu)
{
    uword avail = avail_heap();
    PUSH_ESTK(avail);
    if (trace) printf("HEAPAVAIL\r\n");
}
void sysisugt(M6502 *mpu)
{
    uword a, b;
    word result;

    PULL_ESTK(b);
    PULL_ESTK(a);
    result = a > b ? -1 : 0;
    PUSH_ESTK(result);
    if (trace) printf("ISUGT\r\n");
}
void sysisult(M6502 *mpu)
{
    uword a, b;
    word result;

    PULL_ESTK(b);
    PULL_ESTK(a);
    result = a < b ? -1 : 0;
    PUSH_ESTK(result);
    if (trace) printf("ISULT\r\n");
}
void sysisuge(M6502 *mpu)
{
    uword a, b;
    word result;

    PULL_ESTK(b);
    PULL_ESTK(a);
    result = a >= b ? -1 : 0;
    PUSH_ESTK(result);
    if (trace) printf("ISUGE\r\n");
}
void sysisule(M6502 *mpu)
{
    uword a, b;
    word result;

    PULL_ESTK(b);
    PULL_ESTK(a);
    result = a <= b ? -1 : 0;
    PUSH_ESTK(result);
    if (trace) printf("ISULE\r\n");
}
void syssext(M6502 *mpu)
{
    uword a;

    PULL_ESTK(a);
    a = a & 0x0080 ? (a | 0xFF00) : (a & 0x00FF);
    PUSH_ESTK(a);
}
/*
 * CMDSYS exports
 */
void export_cmdsys(void)
{
    byte dci[16];
    uword defaddr;
    uword machid = alloc_heap(1);
    cmdsys = alloc_heap(23);
    stodci("CMDSYS", dci); add_sym(dci, cmdsys);
    mem_6502[SYSPATH_STR] = strlen(strcat(getcwd((char *)mem_6502 + SYSPATH_BUF, 128), "/sys/"));
    mem_6502[cmdsys + 0] = 0x20; // Version 2.20
    mem_6502[cmdsys + 1] = 0x02;
    mem_6502[cmdsys + 2] = (byte)SYSPATH_STR; // syspath
    mem_6502[cmdsys + 3] = (byte)(SYSPATH_STR >> 8);
    mem_6502[cmdsys + 4] = (byte)CMDLINE_STR; // cmdline
    mem_6502[cmdsys + 5] = (byte)(CMDLINE_STR >> 8);
    defaddr = add_natv(sysexecmod); // sysexecmod
    mem_6502[cmdsys + 6] = (byte)defaddr;
    mem_6502[cmdsys + 7] = (byte)(defaddr >> 8);
    perr = mem_6502 + cmdsys + 16; *perr = 0;
    mem_6502[cmdsys + 17] = 0; // jitcount
    mem_6502[cmdsys + 18] = 0; // jitsize
    mem_6502[cmdsys + 19] = 1; // refcons
    mem_6502[cmdsys + 20] = 0; // devcons
    defaddr = add_natv(syslookuptbl); // syslookuptbl
    mem_6502[cmdsys + 21] = (byte)defaddr;
    mem_6502[cmdsys + 22] = (byte)(defaddr >> 8);
    export_natv("CALL",  syscall6502);
    export_natv("TOUPPER",  systoupper);
    export_natv("STRCPY",  sysstrcpy);
    export_natv("STRCAT",  sysstrcat);
    export_natv("MEMSET",  sysmemset);
    export_natv("MEMCPY",  sysmemcpy);
    export_natv("HEAPMARK",  sysheapmark);
    export_natv("HEAPALLOCALIGN",  sysheapallocalign);
    export_natv("HEAPALLOC",  sysheapalloc);
    export_natv("HEAPRELEASE",  sysheaprelease);
    export_natv("HEAPAVAIL",  sysheapavail);
    export_natv("ISUGT",  sysisugt);
    export_natv("ISUGE",  sysisuge);
    export_natv("ISULT",  sysisult);
    export_natv("ISULE",  sysisule);
    export_natv("SEXT",  syssext);
    //
    // Hack DIVMOD into system
    //
    stodci("DIVMOD", dci); add_sym(dci, heap);
    mem_6502[heap++] = 0x20; // JSR
    mem_6502[heap++] = (byte)VM_INLINE_ENTRY;
    mem_6502[heap++] = (byte)(VM_INLINE_ENTRY >> 8);
    mem_6502[heap++] = 0x36; // DIVMOD
    mem_6502[heap++] = 0x5C; // RET
    //
    // Hack SYSCALL into system (required for PLFORTH)
    //
    stodci("SYSCALL", dci); add_sym(dci, heap);
    mem_6502[heap++] = 0xA5;  // LDA ZP,X
    mem_6502[heap++] = ESTKL;
    mem_6502[heap++] = 0xA5;  // LDY ZP,X
    mem_6502[heap++] = ESTKH;
    mem_6502[heap++] = 0x20; // JSR
    mem_6502[heap++] = (byte)VM_SYSCALL;
    mem_6502[heap++] = (byte)(VM_SYSCALL >> 8);
    //
    // Fake MACHID
    //
    mem_6502[machid] = 0x08; // Apple 1 (NA in ProDOS Tech Ref)
    stodci("MACHID", dci); add_sym(dci, machid);
}
int main(int argc, char **argv)
{
    byte   dci[32];
    char   cmdline[128];
    int    i;
    char  *cmdfile    = "a1plasma.bin";
    char  *modfile = NULL;
    //
    // Init 6502 emulator/VM
    //
    M6502 *mpu = M6502_new(0, mem_6502, 0);
    M6502_reset(mpu);
    M6502_setVector(mpu, IRQ, VM_IRQ_ENTRY); // Dummy address to BRK/IRQ on
    //
    // Parse command line options
    //
    if (--argc)
    {
        argv++;
        while (argc && (*argv)[0] == '-')
        {
            if ((*argv)[1] == 't')
            {
                trace = TRACE;
                mpu->flags |= M6502_SingleStep;
            }
            else if ((*argv)[1] == 's')
            {
                trace = SINGLE_STEP;
                mpu->flags |= M6502_SingleStep;
            }
            argc--;
            argv++;
        }
        if (argc)
        {
            //
            // Everything after the module file is passed in as arguments
            //
            modfile = *argv;
            cmdline[0] = '\0';
            while (argc--)
            {
                strcat(cmdline, *argv++);
                strcat(cmdline," ");
            }
            mem_6502[CMDLINE_STR] = strlen(cmdline) - 1;
            memcpy(mem_6502 + CMDLINE_BUF, &cmdline, mem_6502[CMDLINE_STR]);
        }
    }
    //
    // Run PLVM or Apple1 environment based on passed in modfile
    //
    setRawInput();
    if (modfile)
    {
        //
        // Hooks for BRK, enter VM or native code
        //
        M6502_setCallback(mpu, call, VM_IRQ_ENTRY,      vm_irq);
        M6502_setCallback(mpu, call, VM_INLINE_ENTRY,   vm_indef);
        M6502_setCallback(mpu, call, VM_INDIRECT_ENTRY, vm_iidef);
        M6502_setCallback(mpu, call, VM_EXT_ENTRY,      vm_exdef);
        M6502_setCallback(mpu, call, VM_NATV_ENTRY,     vm_natvdef);
        //
        // 64K - 256 RAM
        //
        mem_6502[FPL]     = 0x00; // Frame pointer = $FF00
        mem_6502[FPH]     = 0xFF;
        mem_6502[PPL]     = 0x00; // Pool pointer = $FF00
        mem_6502[PPH]     = 0xFF;
        mem_6502[0x01FE]  = 0xFF; // Address of $FF (RTN) instruction
        mem_6502[0x01FD]  = 0xFE;
        mpu->registers->s = 0xFC;
        mpu->registers->x = ESTK_DEPTH;
        //
        // Load module from command line - PLVM version
        //
        export_cmdsys();
        export_sysio();
        stodci(modfile, dci);
        if (trace) dump_sym();
        load_mod(mpu, dci);
        if (trace) dump_sym();
    }
    else
    {
        //
        // Load Apple 1 emulation - lib6502 version
        //
        if (!initApple1(mpu, 0x280, cmdfile))
            pfail(cmdfile);
        mpu->registers->pc = 0x0280;
        M6502_exec(mpu);
    }
    M6502_delete(mpu);
    return 0;
}
