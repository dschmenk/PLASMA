#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "plasm.h"
/*
 * Symbol table and fixup information.
 */
#define ID_LEN    32
static int  consts   = 0;
static int  externs  = 0;
static int  globals  = 0;
static int  locals   = 0;
static int  predefs  = 0;
static int  defs     = 0;
static int  asmdefs  = 0;
static int  codetags = 1; // Fix check for break_tag and cont_tag
static int  fixups   = 0;
static char idconst_name[1024][ID_LEN+1];
static int  idconst_value[1024];
static char idglobal_name[1024][ID_LEN+1];
static int  idglobal_type[1024];
static int  idglobal_tag[1024];
static int  localsize = 0;
static char idlocal_name[128][ID_LEN+1];
static int  idlocal_type[128];
static int  idlocal_offset[128];
static char fixup_size[2048];
static int  fixup_type[2048];
static int  fixup_tag[2048];
static t_opseq optbl[256];
static t_opseq *freeop_lst = &optbl[0];
#define FIXUP_BYTE    0x00
#define FIXUP_WORD    0x80
int id_match(char *name, int len, char *id)
{
    if (len == id[0])
    {
        if (len > 16) len = 16;
        while (len--)
        {
            if (name[len] != id[1 + len])
                return (0);
        }
        return (1);
    }
    return (0);
}
int idconst_lookup(char *name, int len)
{
    int i;
    for (i = 0; i < consts; i++)
        if (id_match(name, len, &(idconst_name[i][0])))
            return (i);
    return (-1);
}
int idlocal_lookup(char *name, int len)
{
    int i;
    for (i = 0; i < locals; i++)
        if (id_match(name, len, &(idlocal_name[i][0])))
            return (i);
    return (-1);
}
int idglobal_lookup(char *name, int len)
{
    int i;
    for (i = 0; i < globals; i++)
        if (id_match(name, len, &(idglobal_name[i][0])))
            return (i);
    return (-1);
}
int idconst_add(char *name, int len, int value)
{
    char c = name[len];
    if (consts > 1024)
    {
        printf("Constant count overflow\n");
        return (0);
    }
    name[len] = '\0';
    emit_idconst(name, value);
    name[len] = c;
    idconst_name[consts][0] = len;
    if (len > ID_LEN) len = ID_LEN;
    while (len--)
        idconst_name[consts][1 + len] = name[len];
    idconst_value[consts] = value;
    consts++;
    return (1);
}
int idlocal_add(char *name, int len, int type, int size)
{
    char c = name[len];
    if (localsize > 255)
    {
        printf("Local variable size overflow\n");
        return (0);
    }
    if (idconst_lookup(name, len) > 0)
    {
        parse_error("const/local name conflict\n");
        return (0);
    }
    if (idlocal_lookup(name, len) > 0)
    {
        parse_error("local label already defined\n");
        return (0);
    }
    name[len] = '\0';
    emit_idlocal(name, localsize);
    name[len] = c;
    idlocal_name[locals][0] = len;
    if (len > ID_LEN) len = ID_LEN;
    while (len--)
        idlocal_name[locals][1 + len] = name[len];
    idlocal_type[locals]   = type | LOCAL_TYPE;
    idlocal_offset[locals] = localsize;
    localsize += size;
    locals++;
    return (1);
}
int idglobal_add(char *name, int len, int type, int size)
{
    char c = name[len];
    if (globals > 1024)
    {
        printf("Global variable count overflow\n");
        return (0);
    }
    if (idconst_lookup(name, len) > 0)
    {
        parse_error("const/global name conflict\n");
        return (0);
    }
    if (idglobal_lookup(name, len) > 0)
    {
        parse_error("global label already defined\n");
        return (0);
    }
    name[len] = '\0';
    name[len] = c;
    idglobal_name[globals][0] = len;
    if (len > ID_LEN) len = ID_LEN;
    while (len--)
        idglobal_name[globals][1 + len] = name[len];
    idglobal_type[globals] = type;
    if (!(type & EXTERN_TYPE))
    {
        emit_idglobal(globals, size, name);
        idglobal_tag[globals] = globals;
        globals++;
    }
    else
    {
        printf("\t\t\t\t\t; %s -> X%03d\n", &idglobal_name[globals][1], externs);
        idglobal_tag[globals++] = externs++;
    }
    return (1);
}
int id_add(char *name, int len, int type, int size)
{
    return ((type & LOCAL_TYPE) ? idlocal_add(name, len, type, size) : idglobal_add(name, len, type, size));
}
void idlocal_reset(void)
{
    locals    = 0;
    localsize = 0;
}
int idfunc_add(char *name, int len, int type, int tag)
{
    if (globals > 1024)
    {
        printf("Global variable count overflow\n");
        return (0);
    }
    idglobal_name[globals][0] = len;
    if (len > ID_LEN) len = ID_LEN;
    while (len--)
        idglobal_name[globals][1 + len] = name[len];
    idglobal_type[globals]  = type;
    idglobal_tag[globals++] = tag;
    if (type & EXTERN_TYPE)
        printf("\t\t\t\t\t; %s -> X%03d\n", &idglobal_name[globals - 1][1], tag);
    return (1);
}
int idfunc_set(char *name, int len, int type, int tag)
{
    int i;
    if (((i = idglobal_lookup(name, len)) >= 0) && (idglobal_type[i] & FUNC_TYPE))
    {
        idglobal_tag[i]  = tag;
        idglobal_type[i] = type;
        return (type);
    }
    parse_error("Undeclared identifier");
    return (0);
}
void idglobal_size(int type, int size, int constsize)
{
    if (size > constsize)
        emit_data(0, 0, 0, size - constsize);
    else if (size)
        emit_data(0, 0, 0, size);
}
int id_tag(char *name, int len)
{
    int i;
    if ((i = idlocal_lookup(name, len)) >= 0)
        return (idlocal_offset[i]);
    if ((i = idglobal_lookup(name, len)) >= 0)
        return (idglobal_tag[i]);
    return (-1);
}
int id_const(char *name, int len)
{
    int i;
    if ((i = idconst_lookup(name, len)) >= 0)
        return (idconst_value[i]);
    parse_error("Undeclared constant");
    return (0);
}
int id_type(char *name, int len)
{
    int i;
    if ((i = idconst_lookup(name, len)) >= 0)
        return (CONST_TYPE);
    if ((i = idlocal_lookup(name, len)) >= 0)
        return (idlocal_type[i] | LOCAL_TYPE);
    if ((i = idglobal_lookup(name, len)) >= 0)
        return (idglobal_type[i]);
    parse_error("Undeclared identifier");
    return (0);
}
int tag_new(int type)
{
    if (type & EXTERN_TYPE)
    {
        if (externs > 254)
            parse_error("External variable count overflow\n");
        return (externs++);
    }
    if (type & PREDEF_TYPE)
        return (predefs++);
    if (type & ASM_TYPE)
        return (asmdefs++);
    if (type & DEF_TYPE)
        return (defs++);
    if (type & BRANCH_TYPE)
        return (codetags++);
    return globals++;
}
int fixup_new(int tag, int type, int size)
{
    fixup_tag[fixups]  = tag;
    fixup_type[fixups] = type;
    fixup_size[fixups] = size;
    return (fixups++);
}
/*
 * Emit assembly code.
 */
static const char *DB = ".BYTE";
static const char *DW = ".WORD";
static const char *DS = ".RES";
static char LBL = (char) ':';
char *supper(char *s)
{
    static char su[80];
    int i;
    for (i = 0; s[i]; i++)
        su[i] = toupper(s[i]);
    su[i] = '\0';
    return su;
}
char *tag_string(int tag, int type)
{
    static char str[16];
    char t;

    if (type & EXTERN_TYPE)
        t = 'X';
    else if (type & DEF_TYPE)
        t = 'C';
    else if (type & ASM_TYPE)
        t = 'A';
    else if (type & BRANCH_TYPE)
        t = 'B';
    else if (type & PREDEF_TYPE)
        t = 'P';
    else
        t = 'D';
    sprintf(str, "_%c%03d", t, tag);
    return str;
}
void emit_dci(char *str, int len)
{
    if (len--)
    {
        printf("\t; DCI STRING: %s\n", supper(str));
        printf("\t%s\t$%02X", DB, toupper(*str++) | (len ? 0x80 : 0x00));
        while (len--)
            printf(",$%02X", toupper(*str++) | (len ? 0x80 : 0x00));
        printf("\n");
    }
}
void emit_flags(int flags)
{
    if (flags & ACME)
    {
        DB = "!BYTE";
        DW = "!WORD";
        DS = "!FILL";
        LBL = ' ';
    }
}
void emit_header(void)
{
    int i;

    if (outflags & ACME)
        printf("; ACME COMPATIBLE OUTPUT\n");
    else
        printf("; CA65 COMPATIBLE OUTPUT\n");
    if (outflags & MODULE)
    {
        printf("\t%s\t_SEGEND-_SEGBEGIN\t; LENGTH OF HEADER + CODE/DATA + BYTECODE SEGMENT\n", DW);
        printf("_SEGBEGIN%c\n", LBL);
        printf("\t%s\t$DA7F\t\t\t; MAGIC #\n", DW);
        printf("\t%s\t_SYSFLAGS\t\t\t; SYSTEM FLAGS\n", DW);
        printf("\t%s\t_SUBSEG\t\t\t; BYTECODE SUB-SEGMENT\n", DW);
        printf("\t%s\t_DEFCNT\t\t\t; BYTECODE DEF COUNT\n", DW);
        printf("\t%s\t_INIT\t\t\t; MODULE INITIALIZATION ROUTINE\n", DW);
    }
    else
    {
        printf("\tJMP\t_INIT\t\t\t; MODULE INITIALIZATION ROUTINE\n");
    }
    /*
     * Init free op sequence table
     */
    for (i = 0; i < sizeof(optbl)/sizeof(t_opseq)-1; i++)
        optbl[i].nextop = &optbl[i+1];
    optbl[i].nextop = NULL;
}
void emit_rld(void)
{
    int i;

    printf(";\n; RE-LOCATEABLE DICTIONARY\n;\n");
    /*
     * First emit the bytecode definition entrypoint information.
     */
    for (i = 0; i < globals; i++)
        if (!(idglobal_type[i] & EXTERN_TYPE) && (idglobal_type[i] & DEF_TYPE))
        {
            printf("\t%s\t$02\t\t\t; CODE TABLE FIXUP\n", DB);
            printf("\t%s\t_C%03d\t\t\n", DW, idglobal_tag[i]);
            printf("\t%s\t$00\n", DB);
        }
    /*
     * Now emit the fixup table.
     */
    for (i = 0; i < fixups; i++)
    {
        if (fixup_type[i] & EXTERN_TYPE)
        {
            printf("\t%s\t$%02X\t\t\t; EXTERNAL FIXUP\n", DB, 0x11 + fixup_size[i] & 0xFF);
            printf("\t%s\t_F%03d-_SEGBEGIN\t\t\n", DW, i);
            printf("\t%s\t%d\t\t\t; ESD INDEX\n", DB, fixup_tag[i]);
        }
        else
        {
            printf("\t%s\t$%02X\t\t\t; INTERNAL FIXUP\n", DB, 0x01 + fixup_size[i] & 0xFF);
            printf("\t%s\t_F%03d-_SEGBEGIN\t\t\n", DW, i);
            printf("\t%s\t$00\n", DB);
        }
    }
    printf("\t%s\t$00\t\t\t; END OF RLD\n", DB);
}
void emit_esd(void)
{
    int i;

    printf(";\n; EXTERNAL/ENTRY SYMBOL DICTIONARY\n;\n");
    for (i = 0; i < globals; i++)
    {
        if (idglobal_type[i] & EXTERN_TYPE)
        {
            emit_dci(&idglobal_name[i][1], idglobal_name[i][0]);
            printf("\t%s\t$10\t\t\t; EXTERNAL SYMBOL FLAG\n", DB);
            printf("\t%s\t%d\t\t\t; ESD INDEX\n", DW, idglobal_tag[i]);
        }
        else if  (idglobal_type[i] & EXPORT_TYPE)
        {
            emit_dci(&idglobal_name[i][1], idglobal_name[i][0]);
            printf("\t%s\t$08\t\t\t; ENTRY SYMBOL FLAG\n", DB);
            printf("\t%s\t%s\t\t\n", DW, tag_string(idglobal_tag[i], idglobal_type[i]));
        }
    }
    printf("\t%s\t$00\t\t\t; END OF ESD\n", DB);
}
void emit_trailer(void)
{
    if (!(outflags & BYTECODE_SEG))
        emit_bytecode_seg();
    if (!(outflags & INIT))
        printf("_INIT\t=\t0\n");
    if (!(outflags & SYSFLAGS))
        printf("_SYSFLAGS\t=\t0\n");
    if (outflags & MODULE)
    {
        printf("_DEFCNT\t=\t%d\n", defs);
        printf("_SEGEND%c\n", LBL);
        emit_rld();
        emit_esd();
    }
}
void emit_moddep(char *name, int len)
{
    if (outflags & MODULE)
    {
        if (name)
            emit_dci(name, len);
        else
            printf("\t%s\t$00\t\t\t; END OF MODULE DEPENDENCIES\n", DB);
    }
}
void emit_sysflags(int val)
{
    printf("_SYSFLAGS\t=\t$%04X\t\t; SYSTEM FLAGS\n", val);
    outflags |= SYSFLAGS;
}
void emit_bytecode_seg(void)
{
    if ((outflags & MODULE) && !(outflags & BYTECODE_SEG))
        printf("_SUBSEG%c\t\t\t\t; BYTECODE STARTS\n", LBL);
    outflags |= BYTECODE_SEG;
}
void emit_comment(char *s)
{
    printf("\t\t\t\t\t; %s\n", s);
}
void emit_asm(char *s)
{
    printf("%s\n", s);
}
void emit_idlocal(char *name, int value)
{
    printf("\t\t\t\t\t; %s -> [%d]\n", name, value);
}
void emit_idglobal(int tag, int size, char *name)
{
    if (size == 0)
        printf("_D%03d%c\t\t\t\t\t; %s\n", tag, LBL, name);
    else
        printf("_D%03d%c\t%s\t%d\t\t\t; %s\n", tag, LBL, DS, size, name);
}
void emit_idfunc(int tag, int type, char *name, int is_bytecode)
{
    if (name)
        printf("%s%c\t\t\t\t\t; %s()\n", tag_string(tag, type), LBL, name);
    if (!(outflags & MODULE))
    {
        //printf("%s%c\n", name, LBL);
        if (is_bytecode)
            printf("\tJSR\tINTERP\n");
    }
}
void emit_idconst(char *name, int value)
{
    printf("\t\t\t\t\t; %s = %d\n", name, value);
}
int emit_data(int vartype, int consttype, long constval, int constsize)
{
    int datasize, i;
    unsigned char *str;
    if (consttype == 0)
    {
        datasize = constsize;
        printf("\t%s\t$%02X\n", DS, constsize);
    }
    else if (consttype & STRING_TYPE)
    {
        str = (unsigned char *)constval;
        constsize = *str++;
        datasize = constsize + 1;
        printf("\t%s\t$%02X\n", DB, constsize);
        while (constsize-- > 0)
        {
            printf("\t%s\t$%02X", DB, *str++);
            for (i = 0; i < 7; i++)
            {
                if (constsize-- > 0)
                    printf(",$%02X", *str++);
                else
                    break;
            }
            printf("\n");
        }
    }
    else if (consttype & ADDR_TYPE)
    {
        if (vartype & WORD_TYPE)
        {
            int fixup = fixup_new(constval, consttype, FIXUP_WORD);
            datasize = 2;
            if (consttype & EXTERN_TYPE)
                printf("_F%03d%c\t%s\t0\t\t\t; %s\n", fixup, LBL, DW, tag_string(constval, consttype));
            else
                printf("_F%03d%c\t%s\t%s\n", fixup, LBL, DW, tag_string(constval, consttype));
        }
        else
        {
            int fixup = fixup_new(constval, consttype, FIXUP_BYTE);
            datasize = 1;
            if (consttype & EXTERN_TYPE)
                printf("_F%03d%c\t%s\t0\t\t\t; %s\n", fixup, LBL, DB, tag_string(constval, consttype));
            else
                printf("_F%03d%c\t%s\t%s\n", fixup, LBL, DB, tag_string(constval, consttype));
        }
    }
    else
    {
        if (vartype & WORD_TYPE)
        {
            datasize = 2;
            printf("\t%s\t$%04lX\n", DW, constval & 0xFFFF);
        }
        else
        {
            datasize = 1;
            printf("\t%s\t$%02lX\n", DB, constval & 0xFF);
        }
    }
    return (datasize);
}
void emit_codetag(int tag)
{
    printf("_B%03d%c\n", tag, LBL);
}
void emit_const(int cval)
{
    if (cval == 0)
        printf("\t%s\t$00\t\t\t; ZERO\n", DB);
    else if (cval > 0 && cval < 256)
        printf("\t%s\t$2A,$%02X\t\t\t; CB\t%d\n", DB, cval, cval);
    else
        printf("\t%s\t$2C,$%02X,$%02X\t\t; CW\t%d\n", DB, cval&0xFF,(cval>>8)&0xFF, cval);
}
void emit_conststr(long conststr)
{
    printf("\t%s\t$2E\t\t\t; CS\n", DB);
    emit_data(0, STRING_TYPE, conststr, 0);
}
void emit_lb(void)
{
    printf("\t%s\t$60\t\t\t; LB\n", DB);
}
void emit_lw(void)
{
    printf("\t%s\t$62\t\t\t; LW\n", DB);
}
void emit_llb(int index)
{
    printf("\t%s\t$64,$%02X\t\t\t; LLB\t[%d]\n", DB, index, index);
}
void emit_llw(int index)
{
    printf("\t%s\t$66,$%02X\t\t\t; LLW\t[%d]\n", DB, index, index);
}
void emit_lab(int tag, int offset, int type)
{
    if (type)
    {
        int fixup = fixup_new(tag, type, FIXUP_WORD);
        char *taglbl = tag_string(tag, type);
        printf("\t%s\t$68\t\t\t; LAB\t%s+%d\n", DB, taglbl, offset);
        printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
    }
    else
    {
        printf("\t%s\t$68,$%02X,$%02X\t\t; LAB\t%d\n", DB, offset&0xFF,(offset>>8)&0xFF, offset);
    }
}
void emit_law(int tag, int offset, int type)
{
    if (type)
    {
        int fixup = fixup_new(tag, type, FIXUP_WORD);
        char *taglbl = tag_string(tag, type);
        printf("\t%s\t$6A\t\t\t; LAW\t%s+%d\n", DB, taglbl, offset);
        printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
    }
    else
    {
        printf("\t%s\t$6A,$%02X,$%02X\t\t; LAW\t%d\n", DB, offset&0xFF,(offset>>8)&0xFF, offset);
    }
}
void emit_sb(void)
{
    printf("\t%s\t$70\t\t\t; SB\n", DB);
}
void emit_sw(void)
{
    printf("\t%s\t$72\t\t\t; SW\n", DB);
}
void emit_slb(int index)
{
    printf("\t%s\t$74,$%02X\t\t\t; SLB\t[%d]\n", DB, index, index);
}
void emit_slw(int index)
{
    printf("\t%s\t$76,$%02X\t\t\t; SLW\t[%d]\n", DB, index, index);
}
void emit_dlb(int index)
{
    printf("\t%s\t$6C,$%02X\t\t\t; DLB\t[%d]\n", DB, index, index);
}
void emit_dlw(int index)
{
    printf("\t%s\t$6E,$%02X\t\t\t; DLW\t[%d]\n", DB, index, index);
}
void emit_sab(int tag, int offset, int type)
{
    if (type)
    {
        int fixup = fixup_new(tag, type, FIXUP_WORD);
        char *taglbl = tag_string(tag, type);
        printf("\t%s\t$78\t\t\t; SAB\t%s+%d\n", DB, taglbl, offset);
        printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
    }
    else
    {
        printf("\t%s\t$78,$%02X,$%02X\t\t; SAB\t%d\n", DB, offset&0xFF,(offset>>8)&0xFF, offset);
    }
}
void emit_saw(int tag, int offset, int type)
{
    if (type)
    {
        int fixup = fixup_new(tag, type, FIXUP_WORD);
        char *taglbl = tag_string(tag, type);
        printf("\t%s\t$7A\t\t\t; SAW\t%s+%d\n", DB, taglbl, offset);
        printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
    }
    else
    {
        printf("\t%s\t$7A,$%02X,$%02X\t\t; SAW\t%d\n", DB, offset&0xFF,(offset>>8)&0xFF, offset);
    }
}
void emit_dab(int tag, int offset, int type)
{
    int fixup = fixup_new(tag, type, FIXUP_WORD);
    char *taglbl = tag_string(tag, type);
    printf("\t%s\t$7C\t\t\t; DAB\t%s+%d\n", DB, taglbl, offset);
    printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
}
void emit_daw(int tag, int offset, int type)
{
    int fixup = fixup_new(tag, type, FIXUP_WORD);
    char *taglbl = tag_string(tag, type);
    printf("\t%s\t$7E\t\t\t; DAW\t%s+%d\n", DB, taglbl, offset);
    printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
}
void emit_localaddr(int index)
{
    printf("\t%s\t$28,$%02X\t\t\t; LLA\t[%d]\n", DB, index, index);
}
void emit_globaladdr(int tag, int offset, int type)
{
    int fixup = fixup_new(tag, type, FIXUP_WORD);
    char *taglbl = tag_string(tag, type);
    printf("\t%s\t$26\t\t\t; LA\t%s+%d\n", DB, taglbl, offset);
    printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
}
void emit_indexbyte(void)
{
    printf("\t%s\t$02\t\t\t; IDXB\n", DB);
}
void emit_indexword(void)
{
    printf("\t%s\t$1E\t\t\t; IDXW\n", DB);
}
void emit_brfls(int tag)
{
    printf("\t%s\t$4C\t\t\t; BRFLS\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_brtru(int tag)
{
    printf("\t%s\t$4E\t\t\t; BRTRU\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_brnch(int tag)
{
    printf("\t%s\t$50\t\t\t; BRNCH\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_breq(int tag)
{
    printf("\t%s\t$3C\t\t\t; BREQ\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_brne(int tag)
{
    printf("\t%s\t$3E\t\t\t; BRNE\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_brgt(int tag)
{
    printf("\t%s\t$38\t\t\t; BRGT\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_brlt(int tag)
{
    printf("\t%s\t$3A\t\t\t; BRLT\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_call(int tag, int type)
{
    if (type == CONST_TYPE)
    {
        printf("\t%s\t$54\t\t\t; CALL\t%i\n", DB, tag);
        printf("\t%s\t%i\t\t\n", DW, tag);
    }
    else
    {
        int fixup = fixup_new(tag, type, FIXUP_WORD);
        char *taglbl = tag_string(tag, type);
        printf("\t%s\t$54\t\t\t; CALL\t%s\n", DB, taglbl);
        printf("_F%03d%c\t%s\t%s\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl);
    }
}
void emit_ical(void)
{
    printf("\t%s\t$56\t\t\t; ICAL\n", DB);
}
void emit_leave(void)
{
    if (localsize)
        printf("\t%s\t$5A\t\t\t; LEAVE\n", DB);
    else
        printf("\t%s\t$5C\t\t\t; RET\n", DB);
}
void emit_ret(void)
{
    printf("\t%s\t$5C\t\t\t; RET\n", DB);
}
void emit_enter(int cparams)
{
    if (localsize)
        printf("\t%s\t$58,$%02X,$%02X\t\t; ENTER\t%d,%d\n", DB, localsize, cparams, localsize, cparams);
}
void emit_start(void)
{
    printf("_INIT%c\n", LBL);
    outflags |= INIT;
    defs++;
}
void emit_push_exp(void)
{
    printf("\t%s\t$34\t\t\t; PUSH EXP\n", DB);
}
void emit_pull_exp(void)
{
    printf("\t%s\t$36\t\t\t; PULL EXP\n", DB);
}
void emit_drop(void)
{
    printf("\t%s\t$30\t\t\t; DROP\n", DB);
}
void emit_dup(void)
{
    printf("\t%s\t$32\t\t\t; DUP\n", DB);
}
int emit_unaryop(t_token op)
{
    switch (op)
    {
        case NEG_TOKEN:
            printf("\t%s\t$10\t\t\t; NEG\n", DB);
            break;
        case COMP_TOKEN:
            printf("\t%s\t$12\t\t\t; COMP\n", DB);
            break;
        case LOGIC_NOT_TOKEN:
            printf("\t%s\t$20\t\t\t; NOT\n", DB);
            break;
        case INC_TOKEN:
            printf("\t%s\t$0C\t\t\t; INCR\n", DB);
            break;
        case DEC_TOKEN:
            printf("\t%s\t$0E\t\t\t; DECR\n", DB);
            break;
        case BPTR_TOKEN:
            emit_lb();
            break;
        case WPTR_TOKEN:
            emit_lw();
            break;
        default:
            printf("emit_unaryop(%c) ???\n", op  & 0x7F);
            return (0);
    }
    return (1);
}
int emit_op(t_token op)
{
    switch (op)
    {
        case MUL_TOKEN:
            printf("\t%s\t$06\t\t\t; MUL\n", DB);
            break;
        case DIV_TOKEN:
            printf("\t%s\t$08\t\t\t; DIV\n", DB);
            break;
        case MOD_TOKEN:
            printf("\t%s\t$0A\t\t\t; MOD\n", DB);
            break;
        case ADD_TOKEN:
            printf("\t%s\t$02\t\t\t; ADD\n", DB);
            break;
        case SUB_TOKEN:
            printf("\t%s\t$04\t\t\t; SUB\n", DB);
            break;
        case SHL_TOKEN:
            printf("\t%s\t$1A\t\t\t; SHL\n", DB);
            break;
        case SHR_TOKEN:
            printf("\t%s\t$1C\t\t\t; SHR\n", DB);
            break;
        case AND_TOKEN:
            printf("\t%s\t$14\t\t\t; AND\n", DB);
            break;
        case OR_TOKEN:
            printf("\t%s\t$16\t\t\t; IOR\n", DB);
            break;
        case EOR_TOKEN:
            printf("\t%s\t$18\t\t\t; XOR\n", DB);
            break;
        case EQ_TOKEN:
            printf("\t%s\t$40\t\t\t; ISEQ\n", DB);
            break;
        case NE_TOKEN:
            printf("\t%s\t$42\t\t\t; ISNE\n", DB);
            break;
        case GE_TOKEN:
            printf("\t%s\t$48\t\t\t; ISGE\n", DB);
            break;
        case LT_TOKEN:
            printf("\t%s\t$46\t\t\t; ISLT\n", DB);
            break;
        case GT_TOKEN:
            printf("\t%s\t$44\t\t\t; ISGT\n", DB);
            break;
        case LE_TOKEN:
            printf("\t%s\t$4A\t\t\t; ISLE\n", DB);
            break;
        case LOGIC_OR_TOKEN:
            printf("\t%s\t$22\t\t\t; LOR\n", DB);
            break;
        case LOGIC_AND_TOKEN:
            printf("\t%s\t$24\t\t\t; LAND\n", DB);
            break;
        case COMMA_TOKEN:
            break;
        default:
            return (0);
    }
    return (1);
}
/*
 * New/release sequence ops
 */
t_opseq *new_op(void)
{
    t_opseq* op = freeop_lst;
    if (!op)
    {
        fprintf(stderr, "Compiler out of sequence ops!\n");
        return (NULL);
    }
    freeop_lst = freeop_lst->nextop;
    op->nextop = NULL;
    return (op);
}
void release_op(t_opseq *op)
{
    if (op)
    {
        op->nextop = freeop_lst;
        freeop_lst = op;
    }
}
void release_seq(t_opseq *seq)
{
    t_opseq *op;
    while (seq)
    {
        op = seq;
        seq = seq->nextop;
        /*
         * Free this op
         */
        op->nextop = freeop_lst;
        freeop_lst = op;
    }
}
/*
 * Crunch sequence (peephole optimize)
 */
int crunch_seq(t_opseq **seq)
{
    t_opseq *opnext, *opnextnext;
    t_opseq *op = *seq;
    int crunched = 0;
    int freeops  = 0;
    int shiftcnt;

    while (op && (opnext = op->nextop))
    {
        switch (op->code)
        {
            case CONST_CODE:
                if (op->val == 1)
                {
                    if (opnext->code == BINARY_CODE(ADD_TOKEN))
                    {
                        op->code = INC_CODE;
                        freeops = 1;
                        break;
                    }
                    if (opnext->code == BINARY_CODE(SUB_TOKEN))
                    {
                        op->code = DEC_CODE;
                        freeops = 1;
                        break;
                    }
                }
                switch (opnext->code)
                {
                    case NEG_CODE:
                        op->val = -(op->val);
                        freeops = 1;
                        break;
                    case COMP_CODE:
                        op->val = ~(op->val);
                        freeops = 1;
                        break;
                    case LOGIC_NOT_CODE:
                        op->val = op->val ? 0 : 1;
                        freeops = 1;
                        break;
                    case UNARY_CODE(BPTR_TOKEN):
                    case LB_CODE:
                        op->offsz = op->val;
                        op->code  = LAB_CODE;
                        freeops   = 1;
                        break;
                    case UNARY_CODE(WPTR_TOKEN):
                    case LW_CODE:
                        op->offsz = op->val;
                        op->code  = LAW_CODE;
                        freeops   = 1;
                        break;
                    case SB_CODE:
                        op->offsz = op->val;
                        op->code  = SAB_CODE;
                        freeops   = 1;
                        break;
                    case SW_CODE:
                        op->offsz = op->val;
                        op->code  = SAW_CODE;
                        freeops   = 1;
                        break;
                    case BRFALSE_CODE:
                        if (op->val)
                        {
                            opnextnext = opnext->nextop; // Remove never taken branch
                            if (op == *seq)
                                *seq = opnextnext;
                            opnext->nextop = NULL;
                            release_seq(op);
                            opnext = opnextnext;
                            crunched = 1;
                        }
                        else
                        {
                            op->code = BRNCH_CODE; // Always taken branch
                            op->tag  = opnext->tag;
                            freeops  = 1;
                        }
                        break;
                    case BRTRUE_CODE:
                        if (!op->val)
                        {
                            opnextnext = opnext->nextop; // Remove never taken branch
                            if (op == *seq)
                                *seq = opnextnext;
                            opnext->nextop = NULL;
                            release_seq(op);
                            opnext = opnextnext;
                            crunched = 1;
                        }
                        else
                        {
                            op->code = BRNCH_CODE; // Always taken branch
                            op->tag  = opnext->tag;
                            freeops  = 1;
                        }
                        break;
                    case CONST_CODE: // Collapse constant operation
                        if ((opnextnext = opnext->nextop))
                            switch (opnextnext->code)
                            {
                                case BINARY_CODE(MUL_TOKEN):
                                    op->val *= opnext->val;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(DIV_TOKEN):
                                    op->val /= opnext->val;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(MOD_TOKEN):
                                    op->val %= opnext->val;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(ADD_TOKEN):
                                    op->val += opnext->val;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(SUB_TOKEN):
                                    op->val -= opnext->val;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(SHL_TOKEN):
                                    op->val <<= opnext->val;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(SHR_TOKEN):
                                    op->val >>= opnext->val;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(AND_TOKEN):
                                    op->val &= opnext->val;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(OR_TOKEN):
                                    op->val |= opnext->val;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(EOR_TOKEN):
                                    op->val ^= opnext->val;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(EQ_TOKEN):
                                    op->val = op->val == opnext->val ? 1 : 0;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(NE_TOKEN):
                                    op->val = op->val != opnext->val ? 1 : 0;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(GE_TOKEN):
                                    op->val = op->val >= opnext->val ? 1 : 0;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(LT_TOKEN):
                                    op->val = op->val < opnext->val ? 1 : 0;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(GT_TOKEN):
                                    op->val = op->val > opnext->val ? 1 : 0;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(LE_TOKEN):
                                    op->val = op->val <= opnext->val ? 1 : 0;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(LOGIC_OR_TOKEN):
                                    op->val = op->val || opnext->val ? 1 : 0;
                                    freeops  = 2;
                                    break;
                                case BINARY_CODE(LOGIC_AND_TOKEN):
                                    op->val = op->val && opnext->val ? 1 : 0;
                                    freeops  = 2;
                                    break;
                            }
                        break; // CONST_CODE
                    case BINARY_CODE(MUL_TOKEN):
                        for (shiftcnt = 0; shiftcnt < 16; shiftcnt++)
                        {
                            if (op->val == (1 << shiftcnt))
                            {
                                op->val = shiftcnt;
                                opnext->code = BINARY_CODE(SHL_TOKEN);
                                break;
                            }
                        }
                        break;
                    case BINARY_CODE(DIV_TOKEN):
                       for (shiftcnt = 0; shiftcnt < 16; shiftcnt++)
                        {
                            if (op->val == (1 << shiftcnt))
                            {
                                op->val = shiftcnt;
                                opnext->code = BINARY_CODE(SHR_TOKEN);
                                break;
                            }
                        }
                        break;
                }
                break; // CONST_CODE
            case LADDR_CODE:
                switch (opnext->code)
                {
                    case CONST_CODE:
                        if ((opnextnext = opnext->nextop))
                            switch (opnextnext->code)
                            {
                                case ADD_CODE:
                                case INDEXB_CODE:
                                    op->offsz += opnext->val;
                                    freeops = 2;
                                    break;
                                case INDEXW_CODE:
                                    op->offsz += opnext->val * 2;
                                    freeops = 2;
                                    break;
                            }
                        break;
                    case LB_CODE:
                        op->code  = LLB_CODE;
                        freeops   = 1;
                        break;
                    case LW_CODE:
                        op->code  = LLW_CODE;
                        freeops   = 1;
                        break;
                    case SB_CODE:
                        op->code  = SLB_CODE;
                        freeops   = 1;
                        break;
                    case SW_CODE:
                        op->code  = SLW_CODE;
                        freeops   = 1;
                        break;
                }
                break; // LADDR_CODE
            case GADDR_CODE:
                switch (opnext->code)
                {
                    case CONST_CODE:
                        if ((opnextnext = opnext->nextop))
                            switch (opnextnext->code)
                            {
                                case ADD_CODE:
                                case INDEXB_CODE:
                                    op->offsz += opnext->val;
                                    freeops = 2;
                                    break;
                                case INDEXW_CODE:
                                    op->offsz += opnext->val * 2;
                                    freeops = 2;
                                    break;
                            }
                        break;
                    case LB_CODE:
                        op->code  = LAB_CODE;
                        freeops   = 1;
                        break;
                    case LW_CODE:
                        op->code  = LAW_CODE;
                        freeops   = 1;
                        break;
                    case SB_CODE:
                        op->code  = SAB_CODE;
                        freeops   = 1;
                        break;
                    case SW_CODE:
                        op->code  = SAW_CODE;
                        freeops   = 1;
                        break;
                    case ICAL_CODE:
                        op->code  = CALL_CODE;
                        freeops   = 1;
                        break;
                }
                break; // GADDR_CODE
            case LOGIC_NOT_CODE:
                switch (opnext->code)
                {
                    case BRFALSE_CODE:
                        op->code = BRTRUE_CODE;
                        op->tag  = opnext->tag;
                        freeops  = 1;
                        break;
                    case BRTRUE_CODE:
                        op->code = BRFALSE_CODE;
                        op->tag  = opnext->tag;
                        freeops  = 1;
                        break;
                }
                break; // LOGIC_NOT_CODE
        }
        //
        // Free up crunched ops
        //
        while (freeops)
        {
            op->nextop     = opnext->nextop;
            opnext->nextop = freeop_lst;
            freeop_lst     = opnext;
            opnext         = op->nextop;
            crunched       = 1;
            freeops--;
        }
        op = opnext;
    }
    return (crunched);
}
/*
 * Generate a sequence of code
 */
t_opseq *gen_seq(t_opseq *seq, int opcode, long cval, int tag, int offsz, int type)
{
    t_opseq *op;

    if (!seq)
    {
        op = seq = new_op();
    }
    else
    {
        op = seq;
        while (op->nextop)
            op = op->nextop;
        op->nextop = new_op();
        op = op->nextop;
    }
    op->code  = opcode;
    op->val   = cval;
    op->tag   = tag;
    op->offsz = offsz;
    op->type  = type;
    return (seq);
}
/*
 * Append one sequence to the end of another
 */
t_opseq *cat_seq(t_opseq *seq1, t_opseq *seq2)
{
    t_opseq *op;

    if (!seq1)
        return (seq2);
    for (op = seq1; op->nextop; op = op->nextop);
    op->nextop = seq2;
    return (seq1);
}
/*
 * Emit a sequence of ops
 */
int emit_seq(t_opseq *seq)
{
    t_opseq *op;
    int emitted = 0;

    if (outflags & OPTIMIZE)
        while (crunch_seq(&seq));
    while (seq)
    {
        op = seq;
        switch (op->code)
        {
            case NEG_CODE:
            case COMP_CODE:
            case LOGIC_NOT_CODE:
            case INC_CODE:
            case DEC_CODE:
            case BPTR_CODE:
            case WPTR_CODE:
                emit_unaryop(op->code);
                break;
            case MUL_CODE:
            case DIV_CODE:
            case MOD_CODE:
            case ADD_CODE:
            case SUB_CODE:
            case SHL_CODE:
            case SHR_CODE:
            case AND_CODE:
            case OR_CODE:
            case EOR_CODE:
            case EQ_CODE:
            case NE_CODE:
            case GE_CODE:
            case LT_CODE:
            case GT_CODE:
            case LE_CODE:
            case LOGIC_OR_CODE:
            case LOGIC_AND_CODE:
                emit_op(op->code);
                break;
            case CONST_CODE:
                emit_const(op->val);
                break;
            case STR_CODE:
                emit_conststr(op->val);
                break;
            case LB_CODE:
                emit_lb();
                break;
            case LW_CODE:
                emit_lw();
                break;
            case LLB_CODE:
                emit_llb(op->offsz);
                break;
            case LLW_CODE:
                emit_llw(op->offsz);
                break;
            case LAB_CODE:
                emit_lab(op->tag, op->offsz, op->type);
                break;
            case LAW_CODE:
                emit_law(op->tag, op->offsz, op->type);
                break;
            case SB_CODE:
                emit_sb();
                break;
            case SW_CODE:
                emit_sw();
                break;
            case SLB_CODE:
                emit_slb(op->offsz);
                break;
            case SLW_CODE:
                emit_slw(op->offsz);
                break;
            case DLB_CODE:
                emit_dlb(op->offsz);
                break;
            case DLW_CODE:
                emit_dlw(op->offsz);
                break;
            case SAB_CODE:
                emit_sab(op->tag, op->offsz, op->type);
                break;
            case SAW_CODE:
                emit_saw(op->tag, op->offsz, op->type);
                break;
            case DAB_CODE:
                emit_dab(op->tag, op->offsz, op->type);
                break;
            case DAW_CODE:
                emit_daw(op->tag, op->offsz, op->type);
                break;
            case CALL_CODE:
                emit_call(op->tag, op->type);
                break;
            case ICAL_CODE:
                emit_ical();
                break;
            case LADDR_CODE:
                emit_localaddr(op->offsz);
                break;
            case GADDR_CODE:
                emit_globaladdr(op->tag, op->offsz, op->type);
                break;
            case INDEXB_CODE:
                emit_indexbyte();
                break;
            case INDEXW_CODE:
                emit_indexword();
                break;
            case DROP_CODE:
                emit_drop();
                break;
            case DUP_CODE:
                emit_dup();
                break;
                break;
            case PUSH_EXP_CODE:
                emit_push_exp();
                break;
            case PULL_EXP_CODE:
                emit_pull_exp();
                break;
            case BRNCH_CODE:
                emit_brnch(op->tag);
                break;
            case BRFALSE_CODE:
                emit_brfls(op->tag);
                break;
            case BRTRUE_CODE:
                emit_brtru(op->tag);
                break;
            default:
                return (0);
        }
        emitted++;
        seq = seq->nextop;
        /*
         * Free this op
         */
        op->nextop = freeop_lst;
        freeop_lst = op;
    }
    return (emitted);
}
