#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <ctype.h>

typedef unsigned char  code;
typedef unsigned char  byte;
typedef signed   short word;
typedef unsigned short uword;
typedef unsigned short address;
/*
 * Debug
 */
int show_state = 0;
/*
 * Bytecode memory
 */
#define BYTE_PTR(bp)    ((byte)((bp)[0]))
#define WORD_PTR(bp)    ((word)((bp)[0]  | ((bp)[1] << 8)))
#define UWORD_PTR(bp)   ((uword)((bp)[0] | ((bp)[1] << 8)))
#define TO_UWORD(w) ((uword)((w)))
#define MOD_ADDR    0x1000
#define DEF_CALL    0x0800
#define DEF_CALLSZ  0x0800
#define DEF_ENTRYSZ 6
#define MEM_SIZE    65536
byte mem_data[MEM_SIZE];
uword sp = 0x01FE, fp = 0xFFFF, heap = 0x0200, deftbl = DEF_CALL, lastdef = DEF_CALL;
#define PHA(b)      (mem_data[sp--]=(b))
#define PLA     (mem_data[++sp])
#define EVAL_STACKSZ    16
#define PUSH(v) (*(--esp))=(v)
#define POP     ((word)(*(esp++)))
#define UPOP        ((uword)(*(esp++)))
#define TOS     (esp[0])
word eval_stack[EVAL_STACKSZ];
word *esp = &eval_stack[EVAL_STACKSZ];

#define SYMTBLSZ    1024
#define SYMSZ       16
byte symtbl[SYMTBLSZ];
byte *lastsym = symtbl;
/*
 * Predef.
 */
void interp(code *ip);
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

/*
 * Utility routines.
 *
 * A DCI string is one that has the high bit set for every character except the last.
 * More efficient than C or Pascal strings.
 */
int dcitos(byte *dci, char *str)
{
    int len = 0;
    do
        str[len] = *dci & 0x7F;
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
    return fp - heap;
}
uword alloc_heap(int size)
{
    uword addr = heap;
    heap += size;
    if (heap >= fp)
    {
        printf("Error: heap/frame collision.\n");
        exit (1);
    }
    return addr;
}
uword free_heap(int size)
{
    heap -= size;
    return fp - heap;
}
uword mark_heap(void)
{
    return heap;
}
uword release_heap(uword newheap)
{
    heap = newheap;
    return fp - heap;
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
    *(*last)++ = *dci++;
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
    mem_data[lastdef]     = bank ? 2 : 1;
    mem_data[lastdef + 1] = addr;
    mem_data[lastdef + 2] = addr >> 8;
    return lastdef++;
}
uword def_lookup(byte *cdd, int defaddr)
{
    int i, calldef = 0;
    for (i = 0; cdd[i * 4] == 0x00; i++)
    {
        if ((cdd[i * 4 + 1] | (cdd[i * 4 + 2] << 8)) == defaddr)
        {
            calldef = cdd + i * 4 - mem_data;
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
int load_mod(byte *mod)
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
        strcat(filename, "#FE1000");
    fd = open(filename, O_RDONLY, 0);
    if ((fd > 0) && (len = read(fd, header, 128)) > 0)
    {
        moddep  = header + 1;
        modsize = header[0] | (header[1] << 8);
        magic   = header[2] | (header[3] << 8);
        if (magic == 0x6502)
        {
            /*
             * This is a relocatable bytecode module.
             */
            sysflags = header[4] | (header[5] << 8);
            bytecode = header[6] | (header[7] << 8);
            defcnt   = header[8] | (header[9] << 8);
            init     = header[10] | (header[11] << 8);
            moddep   = header + 12;
            /*
             * Load module dependencies.
             */
            while (*moddep)
            {
                if (lookup_sym(moddep) == 0)
                {
                    if (fd)
                    {
                        close(fd);
                        fd = 0;
                    }
                    load_mod(moddep);
                }
                moddep += dcitos(moddep, string);
            }
            if (fd == 0)
            {
                fd = open(filename, O_RDONLY, 0);
                len = read(fd, header, 128);
            }
        }
        /*
         * Alloc heap space for relocated module (data + bytecode).
         */
        moddep += 1;
        hdrlen  = moddep - header;
        len    -= hdrlen;
        modaddr = mark_heap();
        end     = modaddr + len;
        /*
         * Read in remainder of module into memory for fixups.
         */
        memcpy(mem_data + modaddr, moddep, len);
        while ((len = read(fd, mem_data + end, 4096)) > 0)
            end += len;
        close(fd);
        /*
         * Apply all fixups and symbol import/export.
         */
        modfix    = modaddr - hdrlen + 2; // - MOD_ADDR;
        bytecode += modfix - MOD_ADDR;
        end       = modaddr - hdrlen + modsize + 2;
        rld       = mem_data + end; // Re-Locatable Directory
        esd       = rld;            // Extern+Entry Symbol Directory
        while (*esd != 0x00)        // Scan to end of RLD
            esd += 4;
        esd++;
        cdd = rld;
        if (show_state)
        {
            /*
             * Dump different parts of module.
             */
            printf("Module load addr: $%04X\n", modaddr);
            printf("Module size: %d\n", end - modaddr + hdrlen);
            printf("Module code+data size: %d\n", modsize);
            printf("Module magic: $%04X\n", magic);
            printf("Module sysflags: $%04X\n", sysflags);
            printf("Module bytecode: $%04X\n", bytecode);
            printf("Module def count: $%04X\n", defcnt);
            printf("Module init: $%04X\n", init ? init + modfix - MOD_ADDR : 0);
        }
        /*
         * Add module to symbol table.
         */
        add_sym(mod, modaddr);
        /*
         * Print out the Re-Location Dictionary.
         */
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
                end = rld - mem_data + 4;
            }
            else
            {
                addr = rld[1] | (rld[2] << 8);
                if (addr > 12)
                {
                    addr += modfix;
                    if (rld[0] & 0x80)
                        fixup = (mem_data[addr] | (mem_data[addr + 1] << 8));
                    else
                        fixup = mem_data[addr];
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
                            /*
                             * Replace with call def dictionary.
                             */
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
                        mem_data[addr]     = fixup;
                        mem_data[addr + 1] = fixup >> 8;
                    }
                    else
                    {
                        if (show_state) printf("BYTE");
                        mem_data[addr] = fixup;
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
    /*
     * Reserve heap space for relocated module.
     */
    alloc_heap(end - modaddr);
    /*
     * Call init routine.
     */
    if (init)
    {
        interp(mem_data + init + modfix - MOD_ADDR);
//        release_heap(init + modfix - MOD_ADDR); // Free up init code
        return POP;
    }
    return 0;
}
void interp(code *ip);

void call(uword pc)
{
    unsigned int i, s;
    int a, b;
    char c, sz[64];

    if (show_state)
        printf("\nCall: %s\n", mem_data[pc] ? syslib_exp[mem_data[pc] - 1] : "BYTECODE");
    switch (mem_data[pc++])
    {
        case 0: // BYTECODE in mem_data
            interp(mem_data + (mem_data[pc] + (mem_data[pc + 1] << 8)));
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
            i = mem_data[s++];
            while (i--)
            {
                c = mem_data[s++];
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
                mem_data[0x200 + i] = sz[i];
            mem_data[0x200 + i] = 0;
            mem_data[0x1FF] = i;
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
            printf("\nUnimplemented call code:$%02X\n", mem_data[pc - 1]);
            exit(1);
    }
}

/*
 * OPCODE TABLE
 *
OPTBL   DW CN,CN,CN,CN,CN,CN,CN,CN                                 ; 00 02 04 06 08 0A 0C 0E
        DW CN,CN,CN,CN,CN,CN,CN,CN                                 ; 10 12 14 16 18 1A 1C 1E
        DW MINUS1,BREQ,BRNE,LA,LLA,CB,CW,CS                        ; 20 22 24 26 28 2A 2C 2E
        DW DROP,DROP2,DUP,DIVMOD,ADDI,SUBI,ANDI,ORI                ; 30 32 34 36 38 3A 3C 3E
        DW ISEQ,ISNE,ISGT,ISLT,ISGE,ISLE,BRFLS,BRTRU               ; 40 42 44 46 48 4A 4C 4E
        DW BRNCH,SEL,CALL,ICAL,ENTER,LEAVE,RET,CFFB                ; 50 52 54 56 58 5A 5C 5E
        DW LB,LW,LLB,LLW,LAB,LAW,DLB,DLW                           ; 60 62 64 66 68 6A 6C 6E
        DW SB,SW,SLB,SLW,SAB,SAW,DAB,DAW                           ; 70 72 74 76 78 7A 7C 7E
        DW LNOT,ADD,SUB,MUL,DIV,MOD,INCR,DECR                      ; 80 82 84 86 88 8A 8C 8E
        DW NEG,COMP,BAND,IOR,XOR,SHL,SHR,IDXW                      ; 90 92 94 96 98 9A 9C 9E
        DW BRGT,BRLT,INCBRLE,ADDBRLE,DECBRGE,SUBBRGE,BRAND,BROR    ; A0 A2 A4 A6 A8 AA AC AE
        DW ADDLB,ADDLW,ADDAB,ADDAW,IDXLB,IDXLW,IDXAB,IDXAW         ; B0 B2 B4 B6 B8 BA BC BE
*/
void interp(code *ip)
{
    int val, ea, frmsz, parmcnt;
    code *previp = ip;

    while (1)
    {
        if ((esp - eval_stack) < 0 || (esp - eval_stack) > EVAL_STACKSZ)
        {
            printf("Eval stack over/underflow! - $%04X: $%02X [%d]\n", previp - mem_data, *previp, EVAL_STACKSZ - (esp - eval_stack));
            show_state = 1;
        }
        if (show_state)
        {
            char cmdline[16];
            word *dsp = &eval_stack[EVAL_STACKSZ - 1];
            printf("$%04X: $%02X [ ", ip - mem_data, *ip);
            while (dsp >= esp)
                printf("$%04X ", (*dsp--) & 0xFFFF);
            printf("]\n");
            fgets(cmdline, 15, stdin);
        }
        previp = ip;
        switch (*ip++)
        {
            /*
             * 0x00-0x1F
             */
            case 0x00:
            case 0x02:
            case 0x04:
            case 0x06:
            case 0x08:
            case 0x0A:
            case 0x0C:
            case 0x0E:
            case 0x10:
            case 0x12:
            case 0x14:
            case 0x16:
            case 0x18:
            case 0x1A:
            case 0x1C:
            case 0x1E:
                PUSH(*previp/2);
                break;
                /*
                 * 0x20-0x2F
                 */
            case 0x20: // MINUS 1 : TOS = -1
                PUSH(-1);
                break;
            case 0x22: // BREQ
                val = POP;
                if (val == POP)
                    ip += WORD_PTR(ip) ;
                else
                    ip += 2;
                break;
            case 0x24: // BRNE
                val = POP;
                if (val != POP)
                    ip += WORD_PTR(ip) ;
                else
                    ip += 2;
                break;
            case 0x26: // LA : TOS = @VAR ; equivalent to CW ADDRESSOF(VAR)
                PUSH(WORD_PTR(ip));
                ip += 2;
                break;
            case 0x28: // LLA : TOS = @LOCALVAR ; equivalent to CW FRAMEPTR+OFFSET(LOCALVAR)
                PUSH(fp + BYTE_PTR(ip));
                ip++;
                break;
            case 0x2A: // CB : TOS = CONSTANTBYTE (IP)
                PUSH(BYTE_PTR(ip));
                ip++;
                break;
            case 0x2C: // CW : TOS = CONSTANTWORD (IP)
                PUSH(WORD_PTR(ip));
                ip += 2;
                break;
            case 0x2E: // CS: TOS = CONSTANTSTRING (IP)
                PUSH(ip - mem_data);
                ip += BYTE_PTR(ip) + 1;
                break;
                /*
                 * 0x30-0x3F
                 */
            case 0x30: // DROP : TOS =
                POP;
                break;
            case 0x32: // DROP2 : TOS =, TOS =
                POP;
                POP;
                break;
            case 0x34: // DUP : TOS = TOS
                val = TOS;
                PUSH(val);
                break;
            case 0x36: // DIVMOD
                break;
            case 0x38: // ADDI
                val = POP + BYTE_PTR(ip);
                PUSH(val);
                ip++;
                break;
            case 0x3A: // SUBI
                val = POP - BYTE_PTR(ip);
                PUSH(val);
                ip++;
                break;
            case 0x3C: // ANDI
                val = POP & BYTE_PTR(ip);
                PUSH(val);
                ip++;
                break;
            case 0x3E: // ORI
                val = POP | BYTE_PTR(ip);
                PUSH(val);
                ip++;
                break;
                /*
                 * 0x40-0x4F
                 */
            case 0x40: // ISEQ : TOS = TOS == TOS-1
                val = POP;
                ea  = POP;
                PUSH((ea == val) ? -1 : 0);
                break;
            case 0x42: // ISNE : TOS = TOS != TOS-1
                val = POP;
                ea  = POP;
                PUSH((ea != val) ? -1 : 0);
                break;
            case 0x44: // ISGT : TOS = TOS-1 > TOS
                val = POP;
                ea  = POP;
                PUSH((ea > val) ? -1 : 0);
                break;
            case 0x46: // ISLT : TOS = TOS-1 < TOS
                val = POP;
                ea  = POP;
                PUSH((ea < val) ? -1 : 0);
                break;
            case 0x48: // ISGE : TOS = TOS-1 >= TOS
                val = POP;
                ea  = POP;
                PUSH((ea >= val) ? -1 : 0);
                break;
            case 0x4A: // ISLE : TOS = TOS-1 <= TOS
                val = POP;
                ea  = POP;
                PUSH((ea <= val) ? -1 : 0);
                break;
            case 0x4C: // BRFLS : !TOS ? IP += (IP)
                if (!POP)
                    ip += WORD_PTR(ip) ;
                else
                    ip += 2;
                break;
            case 0x4E: // BRTRU : TOS ? IP += (IP)
                if (POP)
                    ip += WORD_PTR(ip);
                else
                    ip += 2;
                break;
                /*
                 * 0x50-0x5F
                 */
            case 0x50: // BRNCH : IP += (IP)
                ip += WORD_PTR(ip);
                break;
            case 0x52: // SELECT
                val = POP;
                ip += WORD_PTR(ip);
                parmcnt = BYTE_PTR(ip);
                ip++;
                while (parmcnt--)
                {
                    if (WORD_PTR(ip) == val)
                    {
                        ip += 2;
                        ip += WORD_PTR(ip);
                        break;
                    }
                    else
                        ip += 4;                        
                }
                break;
            case 0x54: // CALL : TOFP = IP, IP = (IP) ; call
                call(UWORD_PTR(ip));
                ip += 2;
                break;
            case 0x56: // ICALL : IP = TOS ; indirect call
                ea = UPOP;
                call(ea);
                break;
            case 0x58: // ENTER : NEW FRAME, FOREACH PARAM LOCALVAR = TOS
                frmsz = BYTE_PTR(ip);
                ip++;
                if (show_state)
                    printf("< $%04X: $%04X > ", fp - frmsz, fp);
                fp -= frmsz;
                parmcnt = BYTE_PTR(ip);
                ip++;
                while (parmcnt--)
                {
                    val = POP;
                    mem_data[fp + parmcnt * 2 + 0] = val;
                    mem_data[fp + parmcnt * 2 + 1] = val >> 8;
                    if (show_state)
                        printf("< $%04X: $%04X > ", fp + parmcnt * 2 + 0, mem_data[fp + parmcnt * 2 + 0] | (mem_data[fp + parmcnt * 2 + 1] >> 8));
                }
                if (show_state)
                    printf("\n");
                break;
            case 0x5A: // LEAVE : DEL FRAME, IP = TOFP
                fp += BYTE_PTR(ip);
            case 0x5C: // RET : IP = TOFP
                return;
            case 0x5E: // CFFB : TOS = CONSTANTBYTE(IP) | 0xFF00
                PUSH(BYTE_PTR(ip) | 0xFF00);
                ip++;
                break;
                /*
                 * 0x60-0x6F
                 */
            case 0x60: // LB : TOS = BYTE (TOS)
                ea = TO_UWORD(POP);
                PUSH(mem_data[ea]);
                break;
            case 0x62: // LW : TOS = WORD (TOS)
                ea = UPOP;
                PUSH(mem_data[ea] | (mem_data[ea + 1] << 8));
                break;
            case 0x64: // LLB : TOS = LOCALBYTE [IP]
                PUSH(mem_data[TO_UWORD(fp + BYTE_PTR(ip))]);
                ip++;
                break;
            case 0x66: // LLW : TOS = LOCALWORD [IP]
                ea = TO_UWORD(fp + BYTE_PTR(ip));
                PUSH(mem_data[ea] | (mem_data[ea + 1] << 8));
                ip++;
                break;
            case 0x68: // LAB : TOS = BYTE (IP)
                PUSH(mem_data[UWORD_PTR(ip)]);
                ip += 2;
                break;
            case 0x6A: // LAW : TOS = WORD (IP)
                ea = UWORD_PTR(ip);
                PUSH(mem_data[ea] | (mem_data[ea + 1] << 8));
                ip += 2;
                break;
            case 0x6C: // DLB : TOS = TOS, LOCALBYTE [IP] = TOS
                mem_data[TO_UWORD(fp + BYTE_PTR(ip))] = TOS;
                TOS = TOS & 0xFF;
                ip++;
                break;
            case 0x6E: // DLW : TOS = TOS, LOCALWORD [IP] = TOS
                ea = TO_UWORD(fp + BYTE_PTR(ip));
                mem_data[ea]     = TOS;
                mem_data[ea + 1] = TOS >> 8;
                ip++;
                break;
                /*
                 * 0x70-0x7F
                 */
            case 0x70: // SB : BYTE (TOS-1) = TOS
                ea  = UPOP;
                val = POP;
                mem_data[ea] = val;
                break;
            case 0x72: // SW : WORD (TOS-1) = TOS
                ea  = UPOP;
                val = POP;
                mem_data[ea]     = val;
                mem_data[ea + 1] = val >> 8;
                break;
            case 0x74: // SLB : LOCALBYTE [TOS] = TOS-1
                mem_data[TO_UWORD(fp + BYTE_PTR(ip))] = POP;
                ip++;
                break;
            case 0x76: // SLW : LOCALWORD [TOS] = TOS-1
                ea  = TO_UWORD(fp + BYTE_PTR(ip));
                val = POP;
                mem_data[ea]     = val;
                mem_data[ea + 1] = val >> 8;
                ip++;
                break;
            case 0x78: // SAB : BYTE (IP) = TOS
                mem_data[UWORD_PTR(ip)] = POP;
                ip += 2;
                break;
            case 0x7A: // SAW : WORD (IP) = TOS
                ea = UWORD_PTR(ip);
                val = POP;
                mem_data[ea]     = val;
                mem_data[ea + 1] = val >> 8;
                ip += 2;
                break;
            case 0x7C: // DAB : TOS = TOS, BYTE (IP) = TOS
                mem_data[UWORD_PTR(ip)] = TOS;
                TOS = TOS & 0xFF;
                ip += 2;
                break;
            case 0x7E: // DAW : TOS = TOS, WORD (IP) = TOS
                ea = UWORD_PTR(ip);
                mem_data[ea]     = TOS;
                mem_data[ea + 1] = TOS >> 8;
                ip += 2;
                break;
                /*
                 * 0x080-0x08F
                 */
            case 0x80: // NOT : TOS = !TOS
                TOS = !TOS;
                break;
            case 0x82: // ADD : TOS = TOS + TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea + val);
                break;
            case 0x84: // SUB : TOS = TOS-1 - TOS
                val = POP;
                ea  = POP;
                PUSH(ea - val);
                break;
            case 0x86: // MUL : TOS = TOS * TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea * val);
                break;
            case 0x88: // DIV : TOS = TOS-1 / TOS
                val = POP;
                ea  = POP;
                PUSH(ea / val);
                break;
            case 0x8A: // MOD : TOS = TOS-1 % TOS
                val = POP;
                ea  = POP;
                PUSH(ea % val);
                break;
            case 0x8C: // INCR : TOS = TOS + 1
                TOS++;;
                break;
            case 0x8E: // DECR : TOS = TOS - 1
                TOS--;
                break;
                /*
                 * 0x90-0x9F
                 */
            case 0x90: // NEG : TOS = -TOS
                TOS = -TOS;
                break;
            case 0x92: // COMP : TOS = ~TOS
                TOS = ~TOS;
                break;
            case 0x94: // AND : TOS = TOS & TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea & val);
                break;
            case 0x96: // IOR : TOS = TOS ! TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea | val);
                break;
            case 0x98: // XOR : TOS = TOS ^ TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea ^ val);
                break;
            case 0x9A: // SHL : TOS = TOS-1 << TOS
                val = POP;
                ea  = POP;
                PUSH(ea << val);
                break;
            case 0x9C: // SHR : TOS = TOS-1 >> TOS
                val = POP;
                ea  = POP;
                PUSH(ea >> val);
                break;
            case 0x9E: // IDXW : TOS = TOS * 2 + TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea + val * 2);
                break;
                /*
                 * 0xA0-0xAF
                 */
            case 0xA0: // BRGT : TOS-1 > TOS ? IP += (IP)
                val = POP;
                if (TOS < val)
                    ip += WORD_PTR(ip);
                else
                    ip += 2;
                PUSH(val);
                break;
            case 0xA2: // BRLT : TOS-1 < TOS ? IP += (IP)
                val = POP;
                if (TOS > val)
                    ip += WORD_PTR(ip);
                else
                    ip += 2;
                PUSH(val);
                break;
            case 0xA4: // INCBRLE : TOS = TOS + 1
                val = POP;
                val++;
                if (TOS >= val)
                    ip += WORD_PTR(ip);
                else
                    ip += 2;
                PUSH(val);
                break;
            case 0xA6: // ADDBRLE : TOS = TOS + TOS-1
                val = POP;
                ea  = POP;
                val = ea + val;
                if (TOS >= val)
                    ip += WORD_PTR(ip);
                else
                    ip += 2;
                PUSH(val);
                break;
            case 0xA8: // DECBRGE : TOS = TOS - 1
                val = POP;
                val--;
                if (TOS <= val)
                    ip += WORD_PTR(ip);
                else
                    ip += 2;
                PUSH(val);
                break;
            case 0xAA: // SUBBRGE : TOS = TOS-1 - TOS
                val = POP;
                ea  = POP;
                val = ea - val;
                if (TOS <= val)
                    ip += WORD_PTR(ip);
                else
                    ip += 2;
                PUSH(val);
                break;
            case 0xAC: // BRAND : SHORT CIRCUIT AND
                if (TOS) // EVALUATE RIGHT HAND OF AND
                {
                    POP;
                    ip += 2;
                }
                else // MUST BE FALSE, SKIP RIGHT HAND
                {
                    ip += WORD_PTR(ip);
                }
                break;
            case 0xAE: // BROR : SHORT CIRCUIT OR
                if (!TOS) // EVALUATE RIGHT HAND OF OR
                {
                    POP;
                    ip += 2;
                }
                else // MUST BE TRUE, SKIP RIGHT HAND
                {
                    ip += WORD_PTR(ip);
                }
                break;
                /*
                 * 0xB0-0xBF
                 */
            case 0xB0: // ADDLB : TOS = TOS + LOCALBYTE[IP]
                val = POP + mem_data[TO_UWORD(fp + BYTE_PTR(ip))];
                PUSH(val);
                ip++;
                break;
            case 0xB2: // ADDLW : TOS = TOS + LOCALWORD[IP]
                ea  = TO_UWORD(fp + BYTE_PTR(ip));
                val = POP + (mem_data[ea] | (mem_data[ea + 1] << 8)); 
                PUSH(val);
                ip++;
                break;
            case 0xB4: // ADDAB : TOS = TOS + BYTE[IP]
                val = POP + mem_data[UWORD_PTR(ip)];
                PUSH(val);
                ip  += 2;
                break;
            case 0xB6: // ADDAW : TOS = TOS + WORD[IP]
                ea  = UWORD_PTR(ip);
                val = POP + (mem_data[ea] | (mem_data[ea + 1] << 8));
                PUSH(val);
                ip  += 2;
                break;
            case 0xB8: // IDXLB : TOS = TOS + LOCALBYTE[IP]*2
                val = POP + mem_data[TO_UWORD(fp + BYTE_PTR(ip))] * 2;
                PUSH(val);
                ip++;
                break;
            case 0xBA: // IDXLW : TOS = TOS + LOCALWORD[IP]*2
                ea  = TO_UWORD(fp + BYTE_PTR(ip));
                val = POP + (mem_data[ea] | (mem_data[ea + 1] << 8)) * 2;
                PUSH(val);
                ip++;
                break;
            case 0xBC: // IDXAB : TOS = TOS + BYTE[IP]*2
                val = POP + mem_data[UWORD_PTR(ip)] * 2;
                PUSH(val);
                ip  += 2;
                break;
            case 0xBE: // IDXAW : TOS = TOS + WORD[IP]*2
                ea  = UWORD_PTR(ip);
                val = POP + (mem_data[ea] | (mem_data[ea + 1] << 8)) * 2;
                PUSH(val);
                ip  += 2;
                break;
                /*
                 * Odd codes and everything else are errors.
                 */
            default:
                fprintf(stderr, "Illegal opcode 0x%02X @ 0x%04X\n", ip[-1], ip - mem_data);
                exit(-1);
        }
    }
}

int main(int argc, char **argv)
{
    byte dci[32];
    int i;

    if (--argc)
    {
        argv++;
        if ((*argv)[0] == '-' && (*argv)[1] == 's')
        {
            show_state = 1;
            argc--;
            argv++;
        }
        /*
         * Add default library.
         */
        for (i = 0; syslib_exp[i]; i++)
        {
            mem_data[i] = i;
            stodci(syslib_exp[i], dci);
            add_sym(dci, i+1);
        }
        if (argc)
        {
            stodci(*argv, dci);
            if (show_state) dump_sym();
            load_mod(dci);
            if (show_state) dump_sym();
            argc--;
            argv++;
        }
        if (argc)
        {
            stodci(*argv, dci);
            call(lookup_sym(dci));
        }
        if (esp != &eval_stack[EVAL_STACKSZ])
        {
            printf("Eval stack pointer mismatch at end of execution = %d.\n", EVAL_STACKSZ - (esp - eval_stack));
        }
    }
    return 0;
}
