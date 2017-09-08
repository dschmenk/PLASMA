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
word *esp = eval_stack + EVAL_STACKSZ;

#define SYMTBLSZ    1024
#define SYMSZ       16
#define MODTBLSZ    128
#define MODSZ       16
#define MODLSTSZ    32
byte symtbl[SYMTBLSZ];
byte *lastsym = symtbl;
byte modtbl[MODTBLSZ];
byte *lastmod = modtbl;
/*
 * Predef.
 */
void interp(code *ip);
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
void dump_mod(void)
{
    printf("\nSystem Module Table:\n");
    dump_tbl(modtbl);
}
uword lookup_mod(byte *mod)
{
    return lookup_tbl(mod, modtbl);
}
uword add_mod(byte *mod, int addr)
{
    return add_tbl(mod, addr, &lastmod);
}
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
    for (i = 0; cdd[i * 4] == 0x02; i++)
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
        if (magic == 0xDA7F)
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
                if (lookup_mod(moddep) == 0)
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
        add_mod(mod, modaddr);
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
    char c, sz[64];

    if (show_state)
        printf("\nCall code:$%02X\n", mem_data[pc]);
    switch (mem_data[pc++])
    {
        case 0: // NULL call
            printf("NULL call code\n");
            break;
        case 1: // BYTECODE in mem_code
            //interp(mem_code + (mem_data[pc] + (mem_data[pc + 1] << 8)));
            break;
        case 2: // BYTECODE in mem_data
            interp(mem_data + (mem_data[pc] + (mem_data[pc + 1] << 8)));
            break;
        case 3: // LIBRARY STDLIB::PUTC
            c = POP;
            if (c == 0x0D)
                c = '\n';
            putchar(c);
            break;
        case 4: // LIBRARY STDLIB::PUTS
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
        case 5: // LIBRARY STDLIB::PUTSZ
            s = POP;
            while ((c = mem_data[s++]))
            {
                if (c == 0x0D)
                    c = '\n';
                putchar(c);
            }
            break;
        case 6: // LIBRARY STDLIB::GETC
            PUSH(getchar());
            break;
        case 7: // LIBRARY STDLIB::GETS
            gets(sz);
            for (i = 0; sz[i]; i++)
                mem_data[0x200 + i] = sz[i];
            mem_data[0x200 + i] = 0;
            mem_data[0x1FF] = i;
            PUSH(i);
            break;
        case 8: // LIBRARY STDLIB::PUTNL
            putchar('\n');
            fflush(stdout);
            break;
        default:
            printf("\nBad call code:$%02X\n", mem_data[pc - 1]);
            exit(1);
    }
}

/*
 * OPCODE TABLE
 *
OPTBL:  DW  ZERO,ADD,SUB,MUL,DIV,MOD,INCR,DECR      ; 00 02 04 06 08 0A 0C 0E
        DW  NEG,COMP,AND,IOR,XOR,SHL,SHR,IDXW       ; 10 12 14 16 18 1A 1C 1E
        DW  NOT,LOR,LAND,LA,LLA,CB,CW,CS            ; 20 22 24 26 28 2A 2C 2E
        DW  DROP,DUP,PUSH,PULL,BRGT,BRLT,BREQ,BRNE      ; 30 32 34 36 38 3A 3C 3E
        DW  ISEQ,ISNE,ISGT,ISLT,ISGE,ISLE,BRFLS,BRTRU   ; 40 42 44 46 48 4A 4C 4E
        DW  BRNCH,IBRNCH,CALL,ICAL,ENTER,LEAVE,RET,CFFB  ; 50 52 54 56 58 5A 5C 5E
        DW  LB,LW,LLB,LLW,LAB,LAW,DLB,DLW           ; 60 62 64 66 68 6A 6C 6E
        DW  SB,SW,SLB,SLW,SAB,SAW,DAB,DAW           ; 70 72 74 76 78 7A 7C 7E
*/
void interp(code *ip)
{
    int val, ea, frmsz, parmcnt;

    while (1)
    {
        if (show_state)
        {
            char cmdline[16];
            word *dsp = &eval_stack[EVAL_STACKSZ - 1];
            printf("$%04X: $%02X [ ", ip - mem_data, *ip);
            while (dsp >= esp)
                printf("$%04X ", (*dsp--) & 0xFFFF);
            printf("]\n");
            gets(cmdline);
        }
        switch (*ip++)
        {
        /*
         * 0x00-0x0F
         */
            case 0x00: // ZERO : TOS = 0
                PUSH(0);
                break;
            case 0x02: // ADD : TOS = TOS + TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea + val);
                break;
            case 0x04: // SUB : TOS = TOS-1 - TOS
                val = POP;
                ea  = POP;
                PUSH(ea - val);
                break;
            case 0x06: // MUL : TOS = TOS * TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea * val);
                break;
            case 0x08: // DIV : TOS = TOS-1 / TOS
                val = POP;
                ea  = POP;
                PUSH(ea / val);
                break;
            case 0x0A: // MOD : TOS = TOS-1 % TOS
                val = POP;
                ea  = POP;
                PUSH(ea % val);
                break;
            case 0x0C: // INCR : TOS = TOS + 1
                TOS++;;
                break;
            case 0x0E: // DECR : TOS = TOS - 1
                TOS--;
                break;
                /*
                 * 0x10-0x1F
                 */
            case 0x10: // NEG : TOS = -TOS
                TOS = -TOS;
                break;
            case 0x12: // COMP : TOS = ~TOS
                TOS = ~TOS;
                break;
            case 0x14: // AND : TOS = TOS & TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea & val);
                break;
            case 0x16: // IOR : TOS = TOS ! TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea | val);
                break;
            case 0x18: // XOR : TOS = TOS ^ TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea ^ val);
                break;
            case 0x1A: // SHL : TOS = TOS-1 << TOS
                val = POP;
                ea  = POP;
                PUSH(ea << val);
                break;
            case 0x1C: // SHR : TOS = TOS-1 >> TOS
                val = POP;
                ea  = POP;
                PUSH(ea >> val);
                break;
            case 0x1E: // IDXW : TOS = TOS * 2 + TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea + val * 2);
                break;
                /*
                 * 0x20-0x2F
                 */
            case 0x20: // NOT : TOS = !TOS
                TOS = !TOS;
                break;
            case 0x22: // LOR : TOS = TOS || TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea || val);
                break;
            case 0x24: // LAND : TOS = TOS && TOS-1
                val = POP;
                ea  = POP;
                PUSH(ea && val);
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
            case 0x32: // DUP : TOS = TOS
                val = TOS;
                PUSH(val);
                break;
            case 0x34: // PUSH : TOSP = TOS
                val = esp - eval_stack;
                PHA(val);
                break;
            case 0x36: // PULL : TOS = TOSP
                esp = eval_stack + PLA;
                break;
            case 0x38: // BRGT : TOS-1 > TOS ? IP += (IP)
                val = POP;
                if (TOS > val)
                    ip += WORD_PTR(ip);
                else
                    ip += 2;
                break;
            case 0x3A: // BRLT : TOS-1 < TOS ? IP += (IP)
                val = POP;
                if (TOS < val)
                    ip += WORD_PTR(ip);
                else
                    ip += 2;
                break;
            case 0x3C: // BREQ : TOS == TOS-1 ? IP += (IP)
                val = POP;
                if (TOS == val)
                    ip += WORD_PTR(ip);
                else
                    ip += 2;
                break;
            case 0x3E: // BRNE : TOS != TOS-1 ? IP += (IP)
                val = POP;
                if (TOS != val)
                    ip += WORD_PTR(ip);
                else
                    ip += 2;
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
            case 0x52: // IBRNCH : IP += TOS
                ip += POP;
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
                PHA(frmsz);
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
                fp += PLA;
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
                ip += 2;
                break;
            case 0x7E: // DAW : TOS = TOS, WORD (IP) = TOS
                ea = UWORD_PTR(ip);
                mem_data[ea]     = TOS;
                mem_data[ea + 1] = TOS >> 8;
                ip += 2;
                break;
                /*
                 * Odd codes and everything else are errors.
                 */
            default:
                fprintf(stderr, "Illegal opcode 0x%02X @ 0x%04X\n", ip[-1], ip - mem_data);
        }
    }
}

char *syslib_exp[] = {
    "PUTC",
    "PUTS",
    "PUTSZ",
    "GETC",
    "GETS",
    "PUTLN",
    "MACHID",
    0
};

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
        stodci("CMDSYS", dci);
        add_mod(dci, 0xFFFF);
        for (i = 0; syslib_exp[i]; i++)
        {
            mem_data[i] = i + 3;
            stodci(syslib_exp[i], dci);
            add_sym(dci, i);
        }
        if (argc)
        {
            stodci(*argv, dci);
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
