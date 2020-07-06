#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#define ID_LEN    31
/*
 * Byte code sequence
 */
typedef struct _opseq {
    int code;
    long val;
    int tag;
    int offsz;
    int type;
    struct _opseq *nextop;
} t_opseq;
/*
 * ANS Forth words
 */
typedef struct _forthword {
    char name[ID_LEN+1];
    t_opseq *opseq;
    int tag;
    struct _forthword *nextword;
} t_forthword;
/*
 * Symbol table and fixup information.
 */
int  consts    = 0;
int  externs   = 0;
int  globals   = 0;
int  locals    = 0;
int  localsize = 0;
int  predefs   = 0;
int  defs      = 0;
int  asmdefs   = 0;
int  codetags  = 1; // Fix check for break_tag and cont_tag
int  fixups    = 0;
int  lastglobalsize = 0;
char idconst_name[1024][ID_LEN+1];
int  idconst_value[1024];
char idglobal_name[1024][ID_LEN+1];
int  idglobal_type[1024];
int  idglobal_tag[1024];
char idlocal_name[128][ID_LEN+1];
int  idlocal_type[128];
int  idlocal_offset[128];
char fixup_size[2048];
int  fixup_type[2048];
int  fixup_tag[2048];
int  savelocalsize = 0;
int  savelocals    = 0;
char savelocal_name[128][ID_LEN+1];
int  savelocal_type[128];
int  savelocal_offset[128];
t_opseq optbl[2048];
t_opseq *freeop_lst = &optbl[0];
t_opseq *pending_seq = 0;
t_forthword *dictionary = 0;
#define FIXUP_BYTE    0x00
#define FIXUP_WORD    0x80
#define TOKEN(c)            (0x80|(c))
#define IS_TOKEN(c)         (0x80&(c))
/*
 * Identifier and constant tokens.
 */
#define ID_TOKEN            TOKEN('V')
#define CHAR_TOKEN          TOKEN('Y')
#define INT_TOKEN           TOKEN('Z')
#define FLOAT_TOKEN         TOKEN('F')
#define STRING_TOKEN        TOKEN('S')
/*
 * Keyword tokens.
 */
#define CONST_TOKEN         TOKEN(1)
#define BYTE_TOKEN          TOKEN(2)
#define WORD_TOKEN          TOKEN(3)
#define IF_TOKEN            TOKEN(4)
#define ELSEIF_TOKEN        TOKEN(5)
#define ELSE_TOKEN          TOKEN(6)
#define FIN_TOKEN           TOKEN(7)
#define END_TOKEN           TOKEN(8)
#define WHILE_TOKEN         TOKEN(9)
#define LOOP_TOKEN          TOKEN(10)
#define CASE_TOKEN          TOKEN(11)
#define OF_TOKEN            TOKEN(12)
#define DEFAULT_TOKEN       TOKEN(13)
#define ENDCASE_TOKEN       TOKEN(14)
#define FOR_TOKEN           TOKEN(15)
#define TO_TOKEN            TOKEN(16)
#define DOWNTO_TOKEN        TOKEN(17)
#define STEP_TOKEN          TOKEN(18)
#define NEXT_TOKEN          TOKEN(19)
#define REPEAT_TOKEN        TOKEN(20)
#define UNTIL_TOKEN         TOKEN(21)
#define PREDEF_TOKEN        TOKEN(22)
#define DEF_TOKEN           TOKEN(23)
#define ASM_TOKEN           TOKEN(24)
#define IMPORT_TOKEN        TOKEN(25)
#define EXPORT_TOKEN        TOKEN(26)
#define DONE_TOKEN          TOKEN(27)
#define RETURN_TOKEN        TOKEN(28)
#define BREAK_TOKEN         TOKEN(29)
#define SYSFLAGS_TOKEN      TOKEN(30)
#define STRUC_TOKEN         TOKEN(31)
#define CONTINUE_TOKEN      TOKEN(32)
//#define EVAL_TOKEN          TOKEN(32)
/*
 * Stack double operand operators.
 */
#define SET_TOKEN           TOKEN('=')
#define ADD_TOKEN           TOKEN('+')
#define ADD_SELF_TOKEN      TOKEN('a')
#define SUB_TOKEN           TOKEN('-')
#define SUB_SELF_TOKEN      TOKEN('u')
#define MUL_TOKEN           TOKEN('*')
#define MUL_SELF_TOKEN      TOKEN('m')
#define DIV_TOKEN           TOKEN('/')
#define DIV_SELF_TOKEN      TOKEN('d')
#define MOD_TOKEN           TOKEN('%')
#define OR_TOKEN            TOKEN('|')
#define OR_SELF_TOKEN       TOKEN('o')
#define EOR_TOKEN           TOKEN('^')
#define EOR_SELF_TOKEN      TOKEN('x')
#define AND_TOKEN           TOKEN('&')
#define AND_SELF_TOKEN      TOKEN('n')
#define SHR_TOKEN           TOKEN('R')
#define SHR_SELF_TOKEN      TOKEN('r')
#define SHL_TOKEN           TOKEN('L')
#define SHL_SELF_TOKEN      TOKEN('l')
#define GT_TOKEN            TOKEN('>')
#define GE_TOKEN            TOKEN('H')
#define LT_TOKEN            TOKEN('<')
#define LE_TOKEN            TOKEN('B')
#define NE_TOKEN            TOKEN('U')
#define EQ_TOKEN            TOKEN('E')
#define LOGIC_AND_TOKEN     TOKEN('N')
#define LOGIC_OR_TOKEN      TOKEN('O')
/*
 * Stack single operand operators.
 */
#define NEG_TOKEN           TOKEN('-')
#define COMP_TOKEN          TOKEN('~')
#define LOGIC_NOT_TOKEN     TOKEN('!')
#define LAMBDA_TOKEN        TOKEN('&')
#define INC_TOKEN           TOKEN('P')
#define DEC_TOKEN           TOKEN('K')
#define BPTR_TOKEN          TOKEN('^')
#define WPTR_TOKEN          TOKEN('*')
#define PTRB_TOKEN          TOKEN('b')
#define PTRW_TOKEN          TOKEN('w')
#define POST_INC_TOKEN      TOKEN('p')
#define POST_DEC_TOKEN      TOKEN('k')
#define OPEN_PAREN_TOKEN    TOKEN('(')
#define CLOSE_PAREN_TOKEN   TOKEN(')')
#define OPEN_BRACKET_TOKEN  TOKEN('[')
#define CLOSE_BRACKET_TOKEN TOKEN(']')
/*
 * Misc. tokens.
 */
#define AT_TOKEN            TOKEN('@')
#define DOT_TOKEN           TOKEN('.')
#define COLON_TOKEN         TOKEN(':')
#define POUND_TOKEN         TOKEN('#')
#define COMMA_TOKEN         TOKEN(',')
//#define COMMENT_TOKEN       TOKEN(';')
#define DROP_TOKEN          TOKEN(';')
#define EOL_TOKEN           TOKEN(0)
#define INCLUDE_TOKEN       TOKEN(0x7E)
#define EOF_TOKEN           TOKEN(0x7F)

typedef unsigned char t_token;
#define UNARY_CODE(tkn)  ((tkn)|0x0100)
#define BINARY_CODE(tkn) ((tkn)|0x0200)
#define NEG_CODE    (0x0100|NEG_TOKEN)
#define COMP_CODE   (0x0100|COMP_TOKEN)
#define LOGIC_NOT_CODE (0x0100|LOGIC_NOT_TOKEN)
#define INC_CODE    (0x0100|INC_TOKEN)
#define DEC_CODE    (0x0100|DEC_TOKEN)
#define BPTR_CODE   (0x0100|BPTR_TOKEN)
#define WPTR_CODE   (0x0100|WPTR_TOKEN)
#define MUL_CODE    (0x0200|MUL_TOKEN)
#define DIV_CODE    (0x0200|DIV_TOKEN)
#define MOD_CODE    (0x0200|MOD_TOKEN)
#define ADD_CODE    (0x0200|ADD_TOKEN)
#define SUB_CODE    (0x0200|SUB_TOKEN)
#define SHL_CODE    (0x0200|SHL_TOKEN)
#define SHR_CODE    (0x0200|SHR_TOKEN)
#define AND_CODE    (0x0200|AND_TOKEN)
#define OR_CODE     (0x0200|OR_TOKEN)
#define EOR_CODE    (0x0200|EOR_TOKEN)
#define EQ_CODE     (0x0200|EQ_TOKEN)
#define NE_CODE     (0x0200|NE_TOKEN)
#define GE_CODE     (0x0200|GE_TOKEN)
#define LT_CODE     (0x0200|LT_TOKEN)
#define GT_CODE     (0x0200|GT_TOKEN)
#define LE_CODE     (0x0200|LE_TOKEN)
#define CONST_CODE  0x0300
#define STR_CODE    0x0301
#define LB_CODE     0x0302
#define LW_CODE     0x0303
#define LLB_CODE    0x0304
#define LLW_CODE    0x0305
#define LAB_CODE    0x0306
#define LAW_CODE    0x0307
#define SB_CODE     0x0308
#define SW_CODE     0x0309
#define SLB_CODE    0x030A
#define SLW_CODE    0x030B
#define DLB_CODE    0x030C
#define DLW_CODE    0x030D
#define SAB_CODE    0x030E
#define SAW_CODE    0x030F
#define DAB_CODE    0x0310
#define DAW_CODE    0x0311
#define CALL_CODE   0x0312
#define ICAL_CODE   0x0313
#define LADDR_CODE  0x0314
#define GADDR_CODE  0x0315
#define INDEXB_CODE 0x0316
#define INDEXW_CODE 0x0317
#define DROP_CODE   0x0318
#define DUP_CODE    0x0319
#define ADDI_CODE   0x031A
#define SUBI_CODE   0x031B
#define ANDI_CODE   0x031C
#define ORI_CODE    0x31D
#define BRNCH_CODE  0x0320
#define BRFALSE_CODE 0x0321
#define BRTRUE_CODE 0x0322
#define BREQ_CODE   0x0323
#define BRNE_CODE   0x0324
#define BRAND_CODE  0x0325
#define BROR_CODE   0x0326
#define BRLT_CODE   0x0327
#define BRGT_CODE   0x0328
#define CODETAG_CODE 0x0329
#define NOP_CODE    0x032A
#define ADDLB_CODE  0x0330
#define ADDLW_CODE  0x0331
#define ADDAB_CODE  0x0332
#define ADDAW_CODE  0x0333
#define IDXLB_CODE  0x0334
#define IDXLW_CODE  0x0335
#define IDXAB_CODE  0x0336
#define IDXAW_CODE  0x0337

char *statement, *tokenstr, *scanpos = "", *strpos = "";
t_token scantoken = EOL_TOKEN, prevtoken;
int tokenlen;
long constval;
FILE* inputfile;
char *filename;
int lineno = 0;
FILE* outer_inputfile = NULL;
char* outer_filename;
int outer_lineno;
#define gen_uop(seq,op)     gen_seq(seq,UNARY_CODE(op),0,0,0,0)
#define gen_op(seq,op)      gen_seq(seq,BINARY_CODE(op),0,0,0,0)
#define gen_const(seq,val)  gen_seq(seq,CONST_CODE,val,0,0,0)
#define gen_str(seq,str)    gen_seq(seq,STR_CODE,str,0,0,0)
#define gen_lcladr(seq,idx) gen_seq(seq,LADDR_CODE,0,0,idx,0)
#define gen_gbladr(seq,tag,typ) gen_seq(seq,GADDR_CODE,0,tag,0,typ)
#define gen_idxb(seq)       gen_seq(seq,ADD_CODE,0,0,0,0)
#define gen_idxw(seq)       gen_seq(seq,INDEXW_CODE,0,0,0,0)
#define gen_lb(seq)         gen_seq(seq,LB_CODE,0,0,0,0)
#define gen_lw(seq)         gen_seq(seq,LW_CODE,0,0,0,0)
#define gen_sb(seq)         gen_seq(seq,SB_CODE,0,0,0,0)
#define gen_sw(seq)         gen_seq(seq,SW_CODE,0,0,0,0)
#define gen_icall(seq)      gen_seq(seq,ICAL_CODE,0,0,0,0)
#define gen_drop(seq)       gen_seq(seq,DROP_CODE,0,0,0,0)
#define gen_brand(seq,tag)  gen_seq(seq,BRAND_CODE,0,tag,0,0)
#define gen_bror(seq,tag)   gen_seq(seq,BROR_CODE,0,tag,0,0)
#define gen_brgt(seq,tag)   gen_seq(seq,BRGT_CODE,0,tag,0,0)
#define gen_brlt(seq,tag)   gen_seq(seq,BRLT_CODE,0,tag,0,0)
#define gen_brfls(seq,tag)  gen_seq(seq,BRFALSE_CODE,0,tag,0,0)
#define gen_brtru(seq,tag)  gen_seq(seq,BRTRUE_CODE,0,tag,0,0)
#define gen_brnch(seq,tag)  gen_seq(seq,BRNCH_CODE,0,tag,0,0)
#define gen_codetag(seq,tag) gen_seq(seq, CODETAG_CODE,0,tag,0,0)
#define gen_nop(seq)        gen_seq(seq,NOP_CODE,0,0,0,0)

/*
 * Namespaces
 */
int id_match(char *name, int len, char *id)
{
    if (len == id[0])
    {
        if (len > 16) len = 16;
        while (len--)
        {
            if (toupper(name[len]) != id[1 + len])
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
        {
            if (idglobal_type[i] & EXTERN_TYPE)
                idglobal_type[i] |= ACCESSED_TYPE;
            return (i);
        }
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
    emit_idconst(name, value);
    name[len] = c;
    idconst_name[consts][0] = len;
    if (len > ID_LEN) len = ID_LEN;
    while (len--)
        idconst_name[consts][1 + len] = toupper(name[len]);
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
        idlocal_name[locals][1 + len] = toupper(name[len]);
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
        idglobal_name[globals][1 + len] = toupper(name[len]);
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
void idlocal_save(void)
{
    savelocals    = locals;
    savelocalsize = localsize;
    memcpy(savelocal_name,   idlocal_name,   locals*(ID_LEN+1));
    memcpy(savelocal_type,   idlocal_type,   locals*sizeof(int));
    memcpy(savelocal_offset, idlocal_offset, locals*sizeof(int));
    locals    = 0;
    localsize = 0;
}
void idlocal_restore(void)
{
    locals    = savelocals;
    localsize = savelocalsize;
    memcpy(idlocal_name,   savelocal_name,   locals*(ID_LEN+1));
    memcpy(idlocal_type,   savelocal_type,   locals*sizeof(int));
    memcpy(idlocal_offset, savelocal_offset, locals*sizeof(int));
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
        idglobal_name[globals][1 + len] = toupper(name[len]);
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
void idlocal_size(int size)
{
    localsize += size;
    if (localsize > 255)
    {
        parse_error("Local variable size overflow\n");
    }
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
        printf("\t%s\t$6502\t\t\t; MAGIC #\n", DW);
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
    int i, j;

    printf(";\n; RE-LOCATEABLE DICTIONARY\n;\n");
    /*
     * First emit the bytecode definition entrypoint information.
     */
    /*
    for (i = 0; i < globals; i++)
        if (!(idglobal_type[i] & EXTERN_TYPE) && (idglobal_type[i] & DEF_TYPE))
        {
            printf("\t%s\t$02\t\t\t; CODE TABLE FIXUP\n", DB);
            printf("\t%s\t_C%03d\t\t\n", DW, idglobal_tag[i]);
            printf("\t%s\t$00\n", DB);
        }
    */
    j = outflags & INIT ? defs - 1 : defs;
    for (i = 0; i < j; i++)
    {
        printf("\t%s\t$02\t\t\t; CODE TABLE FIXUP\n", DB);
        printf("\t%s\t_C%03d\t\t\n", DW, i);
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
        if (idglobal_type[i] & ACCESSED_TYPE) // Only refer to accessed externals
        {
            emit_dci(&idglobal_name[i][1], idglobal_name[i][0]);
            printf("\t%s\t$10\t\t\t; EXTERNAL SYMBOL FLAG\n", DB);
            printf("\t%s\t%d\t\t\t; ESD INDEX\n", DW, idglobal_tag[i]);
        }
        else if (idglobal_type[i] & EXPORT_TYPE)
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
        {
            emit_dci(name, len);
            idglobal_add(name, len, EXTERN_TYPE | WORD_TYPE, 2); // Add to symbol table
        }
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
    {
        if (lastglobalsize == 0) // Pad a byte if last label is at end of data segment
            printf("\t%s\t$00\t\t\t; PAD BYTE\n", DB);
        printf("_SUBSEG%c\t\t\t\t; BYTECODE STARTS\n", LBL);
    }
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
    lastglobalsize = size;
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
    emit_pending_seq();
    printf("_B%03d%c\n", tag, LBL);
}
void emit_const(int cval)
{
    emit_pending_seq();
    if ((cval & 0xFFFF) == 0xFFFF)
        printf("\t%s\t$20\t\t\t; MINUS ONE\n", DB);
    else if ((cval & 0xFFF0) == 0x0000)
        printf("\t%s\t$%02X\t\t\t; CN\t%d\n", DB, cval*2, cval);
    else if ((cval & 0xFF00) == 0x0000)
        printf("\t%s\t$2A,$%02X\t\t\t; CB\t%d\n", DB, cval, cval);
    else if ((cval & 0xFF00) == 0xFF00)
        printf("\t%s\t$5E,$%02X\t\t\t; CFFB\t%d\n", DB, cval&0xFF, cval);
    else
        printf("\t%s\t$2C,$%02X,$%02X\t\t; CW\t%d\n", DB, cval&0xFF,(cval>>8)&0xFF, cval);
}
void emit_conststr(long conststr)
{
    printf("\t%s\t$2E\t\t\t; CS\n", DB);
    emit_data(0, STRING_TYPE, conststr, 0);
}
void emit_addi(int cval)
{
    emit_pending_seq();
    printf("\t%s\t$38,$%02X\t\t\t; ADDI\t%d\n", DB, cval, cval);
}
void emit_subi(int cval)
{
    emit_pending_seq();
    printf("\t%s\t$3A,$%02X\t\t\t; SUBI\t%d\n", DB, cval, cval);
}
void emit_andi(int cval)
{
    emit_pending_seq();
    printf("\t%s\t$3C,$%02X\t\t\t; ANDI\t%d\n", DB, cval, cval);
}
void emit_ori(int cval)
{
    emit_pending_seq();
    printf("\t%s\t$3E,$%02X\t\t\t; ORI\t%d\n", DB, cval, cval);
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
void emit_addlb(int index)
{
    printf("\t%s\t$B0,$%02X\t\t\t; ADDLB\t[%d]\n", DB, index, index);
}
void emit_addlw(int index)
{
    printf("\t%s\t$B2,$%02X\t\t\t; ADDLW\t[%d]\n", DB, index, index);
}
void emit_idxlb(int index)
{
    printf("\t%s\t$B8,$%02X\t\t\t; IDXLB\t[%d]\n", DB, index, index);
}
void emit_idxlw(int index)
{
    printf("\t%s\t$BA,$%02X\t\t\t; IDXLW\t[%d]\n", DB, index, index);
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
void emit_addab(int tag, int offset, int type)
{
    if (type)
    {
        int fixup = fixup_new(tag, type, FIXUP_WORD);
        char *taglbl = tag_string(tag, type);
        printf("\t%s\t$B4\t\t\t; ADDAB\t%s+%d\n", DB, taglbl, offset);
        printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
    }
    else
    {
        printf("\t%s\t$B4,$%02X,$%02X\t\t; ADDAB\t%d\n", DB, offset&0xFF,(offset>>8)&0xFF, offset);
    }
}
void emit_addaw(int tag, int offset, int type)
{
    if (type)
    {
        int fixup = fixup_new(tag, type, FIXUP_WORD);
        char *taglbl = tag_string(tag, type);
        printf("\t%s\t$B6\t\t\t; ADDAW\t%s+%d\n", DB, taglbl, offset);
        printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
    }
    else
    {
        printf("\t%s\t$B6,$%02X,$%02X\t\t; ADDAW\t%d\n", DB, offset&0xFF,(offset>>8)&0xFF, offset);
    }
}
void emit_idxab(int tag, int offset, int type)
{
    if (type)
    {
        int fixup = fixup_new(tag, type, FIXUP_WORD);
        char *taglbl = tag_string(tag, type);
        printf("\t%s\t$BC\t\t\t; IDXAB\t%s+%d\n", DB, taglbl, offset);
        printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
    }
    else
    {
        printf("\t%s\t$BC,$%02X,$%02X\t\t; IDXAB\t%d\n", DB, offset&0xFF,(offset>>8)&0xFF, offset);
    }
}
void emit_idxaw(int tag, int offset, int type)
{
    if (type)
    {
        int fixup = fixup_new(tag, type, FIXUP_WORD);
        char *taglbl = tag_string(tag, type);
        printf("\t%s\t$BE\t\t\t; IDXAW\t%s+%d\n", DB, taglbl, offset);
        printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
    }
    else
    {
        printf("\t%s\t$BE,$%02X,$%02X\t\t; IDXAW\t%d\n", DB, offset&0xFF,(offset>>8)&0xFF, offset);
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
    if (type)
    {
        int fixup = fixup_new(tag, type, FIXUP_WORD);
        char *taglbl = tag_string(tag, type);
        printf("\t%s\t$7C\t\t\t; DAB\t%s+%d\n", DB, taglbl, offset);
        printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
    }
    else
        printf("\t%s\t$7C,$%02X,$%02X\t\t; DAB\t%d\n", DB, offset&0xFF,(offset>>8)&0xFF, offset);
}
void emit_daw(int tag, int offset, int type)
{
    if (type)
    {
        int fixup = fixup_new(tag, type, FIXUP_WORD);
        char *taglbl = tag_string(tag, type);
        printf("\t%s\t$7E\t\t\t; DAW\t%s+%d\n", DB, taglbl, offset);
        printf("_F%03d%c\t%s\t%s+%d\t\t\n", fixup, LBL, DW, type & EXTERN_TYPE ? "0" : taglbl, offset);
    }
    else
        printf("\t%s\t$7E,$%02X,$%02X\t\t; DAW\t%d\n", DB, offset&0xFF,(offset>>8)&0xFF, offset);
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
    printf("\t%s\t$82\t\t\t; IDXB\n", DB);
}
void emit_indexword(void)
{
    printf("\t%s\t$9E\t\t\t; IDXW\n", DB);
}
void emit_select(int tag)
{
    emit_pending_seq();
    printf("\t%s\t$52\t\t\t; SEL\n", DB);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_caseblock(int casecnt, int *caseof, int *casetag)
{
    int i;

    if (casecnt < 1 || casecnt > 256)
        parse_error("Switch count under/overflow\n");
    emit_pending_seq();
    printf("\t%s\t$%02lX\t\t\t; CASEBLOCK\n", DB, casecnt & 0xFF);
    for (i = 0; i < casecnt; i++)
    {
        printf("\t%s\t$%04lX\n", DW, caseof[i] & 0xFFFF);
        printf("\t%s\t_B%03d-*\n", DW, casetag[i]);
    }
}
void emit_breq(int tag)
{
    printf("\t%s\t$22\t\t\t; BREQ\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_brne(int tag)
{
    printf("\t%s\t$24\t\t\t; BRNE\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
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
    emit_pending_seq();
    printf("\t%s\t$50\t\t\t; BRNCH\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_brand(int tag)
{
    emit_pending_seq();
    printf("\t%s\t$AC\t\t\t; BRAND\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_bror(int tag)
{
    emit_pending_seq();
    printf("\t%s\t$AE\t\t\t; BROR\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_brgt(int tag)
{
    emit_pending_seq();
    printf("\t%s\t$A0\t\t\t; BRGT\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_brlt(int tag)
{
    emit_pending_seq();
    printf("\t%s\t$A2\t\t\t; BRLT\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_incbrle(int tag)
{
    emit_pending_seq();
    printf("\t%s\t$A4\t\t\t; INCBRLE\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_addbrle(int tag)
{
    emit_pending_seq();
    printf("\t%s\t$A6\t\t\t; ADDBRLE\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_decbrge(int tag)
{
    emit_pending_seq();
    printf("\t%s\t$A8\t\t\t; DECBRGE\t_B%03d\n", DB, tag);
    printf("\t%s\t_B%03d-*\n", DW, tag);
}
void emit_subbrge(int tag)
{
    emit_pending_seq();
    printf("\t%s\t$AA\t\t\t; SUBBRGE\t_B%03d\n", DB, tag);
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
    emit_pending_seq();
    if (localsize)
        printf("\t%s\t$5A,$%02X\t\t\t; LEAVE\t%d\n", DB, localsize, localsize);
    else
        printf("\t%s\t$5C\t\t\t; RET\n", DB);
}
void emit_ret(void)
{
    emit_pending_seq();
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
void emit_drop(void)
{
    emit_pending_seq();
    printf("\t%s\t$30\t\t\t; DROP \n", DB);
}
void emit_drop2(void)
{
    emit_pending_seq();
    printf("\t%s\t$32\t\t\t; DROP2\n", DB);
}
void emit_dup(void)
{
    emit_pending_seq();
    printf("\t%s\t$34\t\t\t; DUP\n", DB);
}
int emit_unaryop(t_token op)
{
    emit_pending_seq();
    switch (op)
    {
        case NEG_TOKEN:
            printf("\t%s\t$90\t\t\t; NEG\n", DB);
            break;
        case COMP_TOKEN:
            printf("\t%s\t$92\t\t\t; COMP\n", DB);
            break;
        case LOGIC_NOT_TOKEN:
            printf("\t%s\t$80\t\t\t; NOT\n", DB);
            break;
        case INC_TOKEN:
            printf("\t%s\t$8C\t\t\t; INCR\n", DB);
            break;
        case DEC_TOKEN:
            printf("\t%s\t$8E\t\t\t; DECR\n", DB);
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
    emit_pending_seq();
    switch (op)
    {
        case MUL_TOKEN:
            printf("\t%s\t$86\t\t\t; MUL\n", DB);
            break;
        case DIV_TOKEN:
            printf("\t%s\t$88\t\t\t; DIV\n", DB);
            break;
        case MOD_TOKEN:
            printf("\t%s\t$8A\t\t\t; MOD\n", DB);
            break;
        case ADD_TOKEN:
            printf("\t%s\t$82\t\t\t; ADD \n", DB);
            break;
        case SUB_TOKEN:
            printf("\t%s\t$84\t\t\t; SUB \n", DB);
            break;
        case SHL_TOKEN:
            printf("\t%s\t$9A\t\t\t; SHL\n", DB);
            break;
        case SHR_TOKEN:
            printf("\t%s\t$9C\t\t\t; SHR\n", DB);
            break;
        case AND_TOKEN:
            printf("\t%s\t$94\t\t\t; AND \n", DB);
            break;
        case OR_TOKEN:
            printf("\t%s\t$96\t\t\t; OR \n", DB);
            break;
        case EOR_TOKEN:
            printf("\t%s\t$98\t\t\t; XOR\n", DB);
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
        case COMMA_TOKEN:
            break;
        default:
            return (0);
    }
    return (1);
}
/*
 * Indicate if an address is (or might be) memory-mapped hardware; used to avoid
 * optimising away accesses to such addresses.
 */
int is_hardware_address(int addr)
{
    // TODO: I think this is reasonable for Apple hardware but I'm not sure.
    // It's a bit too strong for Acorn hardware but code is unlikely to try to
    // read from high addresses anyway, so there's no real harm in not
    // optimising such accesses anyway.
    return addr >= 0xC000;
}
/*
 * Replace all but the first of a series of identical load opcodes by DUP. This
 * doesn't reduce the number of opcodes but does reduce their size in bytes.
 * This is only called on the second optimisation pass because the DUP opcodes
 * may inhibit other peephole optimisations which are more valuable.
 */
int try_dupify(t_opseq *op)
{
    int crunched = 0;
    t_opseq *opn = op->nextop;
    for (; opn; opn = opn->nextop)
    {
        if (op->code != opn->code)
            return crunched;
        switch (op->code)
        {
            case CONST_CODE:
                if (op->val != opn->val)
                    return crunched;
                break;
            case LADDR_CODE:
            case LLB_CODE:
            case LLW_CODE:
                if (op->offsz != opn->offsz)
                    return crunched;
                break;
            case GADDR_CODE:
            case LAB_CODE:
            case LAW_CODE:
                if ((op->tag != opn->tag) || (op->offsz != opn->offsz) /*|| (op->type != opn->type)*/)
                    return crunched;
                break;

            default:
                return crunched;
        }
        opn->code = DUP_CODE;
        crunched  = 1;
    }
    return crunched;
}
/*
 * Crunch sequence (peephole optimize)
 */
int crunch_seq(t_opseq **seq, int pass)
{
    t_opseq *opnext, *opnextnext, *opprev = 0;
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
                    if (opnext->code == BINARY_CODE(SHL_TOKEN))
                    {
                        op->code = DUP_CODE;
                        opnext->code = BINARY_CODE(ADD_TOKEN);
                        break;
                    }
                    if (opnext->code == BINARY_CODE(MUL_TOKEN) || opnext->code == BINARY_CODE(DIV_TOKEN))
                    {
                        freeops = -2;
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
                            freeops = -2; // Remove constant and never taken branch
                        else
                        {
                            op->code = BRNCH_CODE; // Always taken branch
                            op->tag  = opnext->tag;
                            freeops  = 1;
                        }
                        break;
                    case BRTRUE_CODE:
                        if (!op->val)
                            freeops = -2; // Remove constant never taken branch
                        else
                        {
                            op->code = BRNCH_CODE; // Always taken branch
                            op->tag  = opnext->tag;
                            freeops  = 1;
                        }
                        break;
                    case BRGT_CODE:
                        if (opprev && (opprev->code == CONST_CODE) && (op->val <= opprev->val))
                            freeops = 1;
                        break;
                    case BRLT_CODE:
                        if (opprev && (opprev->code == CONST_CODE) && (op->val >= opprev->val))
                            freeops = 1;
                        break;
                    case BROR_CODE:
                        if (!op->val)
                            freeops = -2; // Remove zero constant
                        break;
                    case BRAND_CODE:
                        if (op->val)
                            freeops = -2; // Remove non-zero constant
                        break;
                    case NE_CODE:
                        if (!op->val)
                            freeops = -2; // Remove ZERO:ISNE
                        break;
                    case EQ_CODE:
                        if (!op->val)
                        {
                            op->code = LOGIC_NOT_CODE;
                            freeops = 1;
                        }
                        break;
                    case CONST_CODE:
                        // Collapse constant operation
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
                            }
                        // End of collapse constant operation
                        if ((pass > 0) && (freeops == 0) && (op->val != 0))
                            crunched = try_dupify(op);
                        break; // CONST_CODE
                    case BINARY_CODE(ADD_TOKEN):
                        if (op->val == 0)
                        {
                            freeops = -2;
                        }
                        else if (op->val > 0 && op->val <= 255)
                        {
                            op->code = ADDI_CODE;
                            freeops  = 1;
                        }
                        else if (op->val >= -255 && op->val < 0)
                        {
                            op->code = SUBI_CODE;
                            op->val  = -op->val;
                            freeops  = 1;
                        }
                        break;
                    case BINARY_CODE(SUB_TOKEN):
                        if (op->val == 0)
                        {
                            freeops = -2;
                        }
                        else if (op->val > 0 && op->val <= 255)
                        {
                            op->code = SUBI_CODE;
                            freeops  = 1;
                        }
                        else if (op->val >= -255 && op->val < 0)
                        {
                            op->code = ADDI_CODE;
                            op->val  = -op->val;
                            freeops  = 1;
                        }
                        break;
                    case BINARY_CODE(AND_TOKEN):
                        if (op->val >= 0 && op->val <= 255)
                        {
                            op->code = ANDI_CODE;
                            freeops  = 1;
                        }
                        break;
                    case BINARY_CODE(OR_TOKEN):
                        if (op->val == 0)
                        {
                            freeops = -2;
                        }
                        else if (op->val > 0 && op->val <= 255)
                        {
                            op->code = ORI_CODE;
                            freeops  = 1;
                        }
                        break;
                    case BINARY_CODE(MUL_TOKEN):
                        if (op->val == 0)
                        {
                            op->code = DROP_CODE;
                            opnext->code = CONST_CODE;
                            opnext->val = 0;
                        }
                        else if (op->val == 2)
                        {
                            op->code = DUP_CODE;
                            opnext->code = BINARY_CODE(ADD_TOKEN);
                        }
                        else
                        {
                            //
                            // Constants 0, 1 and 2 handled above
                            //
                            for (shiftcnt = 2; shiftcnt < 16; shiftcnt++)
                            {
                                if (op->val == (1 << shiftcnt))
                                {
                                    op->val = shiftcnt;
                                    opnext->code = BINARY_CODE(SHL_TOKEN);
                                    break;
                                }
                            }
                        }
                        break;
                    case BINARY_CODE(DIV_TOKEN):
                        //
                        // Constant 1 handled above
                        //
                        for (shiftcnt = 1; shiftcnt < 16; shiftcnt++)
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
                if ((pass > 0) && (freeops == 0))
                    crunched = try_dupify(op);
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
                if ((pass > 0) && (freeops == 0))
                    crunched = try_dupify(op);
                break; // GADDR_CODE
            case LLB_CODE:
                if ((opnext->code == ADD_CODE) || (opnext->code == INDEXB_CODE))
                {
                    op->code = ADDLB_CODE;
                    freeops  = 1;
                }
                else if (opnext->code == INDEXW_CODE)
                {
                    op->code = IDXLB_CODE;
                    freeops  = 1;
                }
                else if (pass > 0)
                    crunched = try_dupify(op);
                break; // LLB_CODE
            case LLW_CODE:
                // LLW [n]:CB 8:SHR -> LLB [n+1]
                if ((opnext->code == CONST_CODE) && (opnext->val == 8))
                {
                    if ((opnextnext = opnext->nextop))
                    {
                        if (opnextnext->code == SHR_CODE)
                        {
                            op->code = LLB_CODE;
                            op->offsz++;
                            freeops = 2;
                            break;
                        }
                    }
                }
                else if ((opnext->code == ADD_CODE) || (opnext->code == INDEXB_CODE))
                {
                    op->code = ADDLW_CODE;
                    freeops  = 1;
                }
                else if (opnext->code == INDEXW_CODE)
                {
                    op->code = IDXLW_CODE;
                    freeops  = 1;
                }
                else if (pass > 0)
                    crunched = try_dupify(op);
                break; // LLW_CODE
            case LAB_CODE:
                if ((opnext->code == ADD_CODE) || (opnext->code == INDEXB_CODE))
                {
                    op->code = ADDAB_CODE;
                    freeops  = 1;
                }
                else if (opnext->code == INDEXW_CODE)
                {
                    op->code = IDXAB_CODE;
                    freeops  = 1;
                }
                else if ((pass > 0) && (op->type || !is_hardware_address(op->offsz)))
                    crunched = try_dupify(op);
                break; // LAB_CODE
            case LAW_CODE:
                // LAW x:CB 8:SHR -> LAB x+1
                if ((opnext->code == CONST_CODE) && (opnext->val == 8))
                {
                    if ((opnextnext = opnext->nextop))
                    {
                        if (opnextnext->code == SHR_CODE)
                        {
                            op->code = LAB_CODE;
                            op->offsz++;
                            freeops = 2;
                            break;
                        }
                    }
                }
                else if ((opnext->code == ADD_CODE) || (opnext->code == INDEXB_CODE))
                {
                    op->code = ADDAW_CODE;
                    freeops  = 1;
                }
                else if (opnext->code == INDEXW_CODE)
                {
                    op->code = IDXAW_CODE;
                    freeops  = 1;
                }
                else if ((pass > 0) && (op->type || !is_hardware_address(op->offsz)))
                    crunched = try_dupify(op);
                break; // LAW_CODE
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
            case EQ_CODE:
                switch (opnext->code)
                {
                    case BRFALSE_CODE:
                        op->code = BRNE_CODE;
                        op->tag  = opnext->tag;
                        freeops  = 1;
                        break;
                    case BRTRUE_CODE:
                        op->code = BREQ_CODE;
                        op->tag  = opnext->tag;
                        freeops  = 1;
                        break;
                }
                break; // EQ_CODE
            case NE_CODE:
                switch (opnext->code)
                {
                    case BRFALSE_CODE:
                        op->code = BREQ_CODE;
                        op->tag  = opnext->tag;
                        freeops  = 1;
                        break;
                    case BRTRUE_CODE:
                        op->code = BRNE_CODE;
                        op->tag  = opnext->tag;
                        freeops  = 1;
                        break;
                }
                break; // NE_CODE
            case SLB_CODE:
                if ((opnext->code == LLB_CODE) && (op->offsz == opnext->offsz))
                {
                    op->code = DLB_CODE;
                    freeops = 1;
                }
                break; // SLB_CODE
            case SLW_CODE:
                if ((opnext->code == LLW_CODE) && (op->offsz == opnext->offsz))
                {
                    op->code = DLW_CODE;
                    freeops = 1;
                }
                break; // SLW_CODE
            case SAB_CODE:
                if ((opnext->code == LAB_CODE) && (op->tag == opnext->tag) &&
                    (op->offsz == opnext->offsz) && (op->type == opnext->type))
                {
                    op->code = DAB_CODE;
                    freeops = 1;
                }
                break; // SAB_CODE
            case SAW_CODE:
                if ((opnext->code == LAW_CODE) && (op->tag == opnext->tag) &&
                    (op->offsz == opnext->offsz) && (op->type == opnext->type))
                {
                    op->code = DAW_CODE;
                    freeops = 1;
                }
                break; // SAW_CODE
        }
        //
        // Free up crunched ops. If freeops is positive we free up that many ops
        // *after* op; if it's negative, we free up abs(freeops) ops *starting
        // with* op.
        if (freeops < 0)
        {
            freeops = -freeops;
            // If op is at the start of the sequence, we treat this as a special
            // case.
            if (op == *seq)
            {
                for (; freeops > 0; --freeops)
                {
                    opnext = op->nextop;
                    release_op(op);
                    op = *seq = opnext;
                }
                crunched = 1;
            }
            // Otherwise we just move op back to point to the previous op and
            // let the following loop remove the required number of ops.
            else
            {
                op      = opprev;
                opnext  = op->nextop;
            }
        }
        while (freeops)
        {
            op->nextop     = opnext->nextop;
            opnext->nextop = freeop_lst;
            freeop_lst     = opnext;
            opnext         = op->nextop;
            crunched       = 1;
            freeops--;
        }
        opprev = op;
        op = opnext;
    }
    return (crunched);
}

/*
 * Emit a sequence of ops (into the pending sequence)
 */
int emit_seq(t_opseq *seq)
{
    t_opseq *op;
    int emitted = 0;
    int string = 0;
    for (op = seq; op; op = op->nextop)
    {
        if (op->code == STR_CODE)
            string = 1;
        emitted++;
    }
    pending_seq = cat_seq(pending_seq, seq);
    // The source code comments in the output are much more logical if we don't
    // merge multiple sequences together. There's no value in doing this merging
    // if we're not optimizing, and we optionally allow it to be prevented even
    // when we are optimizing by specifing the -N (NO_COMBINE) flag.
    //
    // We must also force output if the sequence includes a CS opcode, as the
    // associated 'constant' is only temporarily valid.
    if (!(outflags & OPTIMIZE) || (outflags & NO_COMBINE) || string)
        return emit_pending_seq();
    return (emitted);
}
/*
 * Emit the pending sequence
 */
int emit_pending_seq()
{
    // This is called by some of the emit_*() functions to ensure that any
    // pending ops are emitted before they emit their own op when they are
    // called from the parser. However, this function itself calls some of those
    // emit_*() functions to emit instructions from the pending sequence, which
    // would cause an infinite loop if we weren't careful. We therefore set
    // pending_seq to null on entry and work with a local copy, so if this
    // function calls back into itself it is a no-op.
    if (!pending_seq)
        return 0;
    t_opseq *local_pending_seq = pending_seq;
    pending_seq = 0;

    t_opseq *op;
    int emitted = 0;

    if (outflags & OPTIMIZE)
    {
        int pass;
        for (pass = 0; pass < 2; pass++)
            while (crunch_seq(&local_pending_seq, pass));
    }
    while (local_pending_seq)
    {
        op = local_pending_seq;
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
                emit_op(op->code);
                break;
            case CONST_CODE:
                emit_const(op->val);
                break;
            case STR_CODE:
                emit_conststr(op->val);
                break;
            case ADDI_CODE:
                emit_addi(op->val);
                break;
            case SUBI_CODE:
                emit_subi(op->val);
                break;
            case ANDI_CODE:
                emit_andi(op->val);
                break;
            case ORI_CODE:
                emit_ori(op->val);
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
            case ADDLB_CODE:
                emit_addlb(op->offsz);
                break;
            case ADDLW_CODE:
                emit_addlw(op->offsz);
                break;
            case IDXLB_CODE:
                emit_idxlb(op->offsz);
                break;
            case IDXLW_CODE:
                emit_idxlw(op->offsz);
                break;
            case LAB_CODE:
                emit_lab(op->tag, op->offsz, op->type);
                break;
            case LAW_CODE:
                emit_law(op->tag, op->offsz, op->type);
                break;
            case ADDAB_CODE:
                emit_addab(op->tag, op->offsz, op->type);
                break;
            case ADDAW_CODE:
                emit_addaw(op->tag, op->offsz, op->type);
                break;
            case IDXAB_CODE:
                emit_idxab(op->tag, op->offsz, op->type);
                break;
            case IDXAW_CODE:
                emit_idxaw(op->tag, op->offsz, op->type);
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
            case BRNCH_CODE:
                emit_brnch(op->tag);
                break;
            case BRAND_CODE:
                emit_brand(op->tag);
                break;
            case BROR_CODE:
                emit_bror(op->tag);
                break;
            case BREQ_CODE:
                emit_breq(op->tag);
                break;
            case BRNE_CODE:
                emit_brne(op->tag);
                break;
            case BRFALSE_CODE:
                emit_brfls(op->tag);
                break;
            case BRTRUE_CODE:
                emit_brtru(op->tag);
                break;
            case BRGT_CODE:
                emit_brgt(op->tag);
                break;
            case BRLT_CODE:
                emit_brlt(op->tag);
                break;
            case CODETAG_CODE:
                printf("_B%03d%c\n", op->tag, LBL);
                break;
            case NOP_CODE:
                break;
            default:
                return (0);
        }
        emitted++;
        local_pending_seq = local_pending_seq->nextop;
        /*
         * Free this op
         */
        op->nextop = freeop_lst;
        freeop_lst = op;
    }
    return (emitted);
}
/*
 * New/release/dup sequence ops
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
t_opseq *dup_seq(t_opseq *seq)
{
    t_opseq *newseq, *op;
    
    newseq = NULL;
    if (seq)
    {
        do
        {
            if (newseq)
            {
                op->nextop = new_op();
                op = op->nextop;
            }
            else
                newseq = op = new_op();
            op->code   = seq->code;
            op->val    = seq->val;
            op->tag    = seq->tag;
            op->offsz  = seq->offsz;
            op->type   = seq->type;
            op->nextop = NULL;
        } while ((seq = seq->nextop));
    }
    return newseq;
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
//
// Dictionary operations
//
t_forthword *search_dict(char *tokenstr)
{
    t_forthword *match;
    
    match = dictionary;
    while (match && strcmp(match->name, tokenstr))
        match = match->nextword;
    return match;
}
t_forth *add_dict(char *wordname, t_opseq *wordseq, int wordlabel)
{
    t_forthword *newword;
    
    newword           = malloc(sizeof(t_forthword);
    newword->nextword = dictionary;
    dictionary        = newword;
    strcpy(newword->name, wordname);
    newword->opseq = wordseq;
    newword->label = wordlabel;
    return newword;
}
void init_dict(void)
{
    //
    // Build baseline dicionary
    //
    add_dict("+", gen_op(NULL, ADD_TOKEN), 0);
    add_dict("-", gen_op(NULL, SUB_TOKEN), 0);
    add_dict("*", gen_op(NULL, MUL_TOKEN), 0);
    add_dict("/", gen_op(NULL, DIV_TOKEN), 0);
}
void parse_error(const char *errormsg)
{
    char *error_carrot = statement;

    fprintf(stderr, "\n%s %4d: %s\n%*s       ", filename, lineno, statement, (int)strlen(filename), "");
    for (error_carrot = statement; error_carrot != tokenstr; error_carrot++)
        putc(*error_carrot == '\t' ? '\t' : ' ', stderr);
    fprintf(stderr, "^\nError: %s\n", errormsg);
    exit(1);
}
void parse_warn(const char *warnmsg)
{
    if (outflags & WARNINGS)
    {
        char *error_carrot = statement;

        fprintf(stderr, "\n%s %4d: %s\n%*s       ", filename, lineno, statement, (int)strlen(filename), "");
        for (error_carrot = statement; error_carrot != tokenstr; error_carrot++)
            putc(*error_carrot == '\t' ? '\t' : ' ', stderr);
        fprintf(stderr, "^\nWarning: %s\n", warnmsg);
    }
}
int hexdigit(char ch)
{
    ch = toupper(ch);
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    else if (ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    else
        return -1;
}

t_token scan(void)
{
    prevtoken = scantoken;
    /*
     * Skip whitespace.
     */
    while (*scanpos && (*scanpos == ' ' || *scanpos == '\t')) scanpos++;
    tokenstr = scanpos;
    /*
     * Scan for token based on first character.
     */
    if (scantoken == EOF_TOKEN)
        ;
    else if (*scanpos == '\0' || *scanpos == '\n' || *scanpos == ';')
        scantoken = EOL_TOKEN;
    else if ((scanpos[0] >= 'a' && scanpos[0] <= 'z')
          || (scanpos[0] >= 'A' && scanpos[0] <= 'Z')
          || (scanpos[0] == '_'))
    {
        /*
         * ID,  either variable name or reserved word.
         */
        int keypos = 0, matchpos = 0;

        do
        {
            scanpos++;
        }
        while ((*scanpos >= 'a' && *scanpos <= 'z')
            || (*scanpos >= 'A' && *scanpos <= 'Z')
            || (*scanpos == '_')
            || (*scanpos >= '0' && *scanpos <= '9'));
        scantoken = ID_TOKEN;
        tokenlen = scanpos - tokenstr;
        /*
         * Search for matching keyword.
         */
        while (keywords[keypos] != EOL_TOKEN)
        {
            while (keywords[keypos + 1 + matchpos] == toupper(tokenstr[matchpos]))
                matchpos++;
            if (IS_TOKEN(keywords[keypos + 1 + matchpos]) && (matchpos == tokenlen))
            {
                /*
                 * A match.
                 */
                scantoken = keywords[keypos];
                break;
            }
            else
            {
                /*
                 * Find next keyword.
                 */
                keypos  += matchpos + 1;
                matchpos = 0;
                while (!IS_TOKEN(keywords[keypos])) keypos++;
            }
        }
    }
    else if (scanpos[0] >= '0' && scanpos[0] <= '9')
    {
        /*
         * Number constant.
         */
        for (constval = 0; *scanpos >= '0' && *scanpos <= '9'; scanpos++)
            constval = constval * 10 + *scanpos - '0';
        scantoken = INT_TOKEN;
    }
    else if (scanpos[0] == '$')
    {
        /*
         * Hexadecimal constant.
         */
        constval = 0;
        while (scanpos++)
        {
            if (hexdigit(*scanpos) >= 0)
                constval = constval * 16 + hexdigit(*scanpos);
            else
                break;
        }
        scantoken = INT_TOKEN;
    }
    else if (scanpos[0] == '\'')
    {
        /*
         * Character constant.
         */
        scantoken = CHAR_TOKEN;
        if (scanpos[1] != '\\')
        {
            constval =  scanpos[1];
            if (scanpos[2] != '\'')
            {
                parse_error("Bad character constant");
                return (-1);
            }
            scanpos += 3;
        }
        else
        {
            switch (scanpos[2])
            {
                case 'n':
                    constval = 0x0D;
                    break;
                case 'r':
                    constval = 0x0A;
                    break;
                case 't':
                    constval = '\t';
                    break;
                case '\'':
                    constval = '\'';
                    break;
                case '\\':
                    constval = '\\';
                    break;
                case '0':
                    constval = '\0';
                    break;
                default:
                    parse_error("Bad character constant");
                    return (-1);
            }
            if (scanpos[3] != '\'')
            {
                parse_error("Bad character constant");
                return (-1);
            }
            scanpos += 4;
        }
    }
    else if (scanpos[0] == '\"') // Hack for string quote char in case we have to rewind later
    {
        int scanoffset;
        /*
         * String constant.
         */
        scantoken   = STRING_TOKEN;
        constval    = (long)strpos++;
        scanpos++;
        while (*scanpos &&  *scanpos != '\"')
        {
            if (*scanpos == '\\')
            {
                scanoffset = 2;
                switch (scanpos[1])
                {
                    case 'n':
                        *strpos++ = 0x0D;
                        break;
                    case 'r':
                        *strpos++ = 0x0A;
                        break;
                    case 't':
                        *strpos++ = '\t';
                        break;
                    case '\'':
                        *strpos++ = '\'';
                        break;
                    case '\"':
                        *strpos++ = '\"';
                        break;
                    case '\\':
                        *strpos++ = '\\';
                        break;
                    case '0':
                        *strpos++ = '\0';
                        break;
                    case '$':
                        if (hexdigit(scanpos[2]) < 0 || hexdigit(scanpos[3]) < 0) {
                            parse_error("Bad string constant");
                            return (-1);
                        }
                        *strpos++ = hexdigit(scanpos[2]) * 16 + hexdigit(scanpos[3]);
                        scanoffset = 4;
                        break;
                    default:
                        parse_error("Bad string constant");
                        return (-1);
                }
                scanpos += scanoffset;
            }
            else
                *strpos++ = *scanpos++;
        }
        if (!*scanpos)
        {
            parse_error("Unterminated string");
            return (-1);
        }
        *((unsigned char *)constval) = (long)strpos - constval - 1;
        *strpos++ = '\0';
        scanpos++;
    }
    else
    {
        /*
         * Potential two and three character tokens.
         */
        switch (scanpos[0])
        {
            case '>':
                if (scanpos[1] == '>')
                {
                    scantoken = SHR_TOKEN;
                    scanpos += 2;
                }
                else if (scanpos[1] == '=')
                {
                    scantoken = GE_TOKEN;
                    scanpos += 2;
                }
                else
                {
                    scantoken = GT_TOKEN;
                    scanpos++;
                }
                break;
            case '<':
                if (scanpos[1] == '<')
                {
                    scantoken = SHL_TOKEN;
                    scanpos += 2;
                }
                else if (scanpos[1] == '=')
                {
                    scantoken = LE_TOKEN;
                    scanpos += 2;
                }
                else if (scanpos[1] == '>')
                {
                    scantoken = NE_TOKEN;
                    scanpos += 2;
                }
                else
                {
                    scantoken = LT_TOKEN;
                    scanpos++;
                }
                break;
            case '=':
                if (scanpos[1] == '=')
                {
                    scantoken = EQ_TOKEN;
                    scanpos += 2;
                }
                else if (scanpos[1] == '>')
                {
                    scantoken = PTRW_TOKEN;
                    scanpos += 2;
                }
                else
                {
                    scantoken = SET_TOKEN;
                    scanpos++;
                }
                break;
            case '+':
                if (scanpos[1] == '+')
                {
                    scantoken = INC_TOKEN;
                    scanpos += 2;
                }
                else
                {
                    scantoken = ADD_TOKEN;
                    scanpos++;
                }
                break;
            case '-':
                if (scanpos[1] == '-')
                {
                    scantoken = DEC_TOKEN;
                    scanpos += 2;
                }
                else if (scanpos[1] == '>')
                {
                    scantoken = PTRB_TOKEN;
                    scanpos += 2;
                }
                else
                {
                    scantoken = SUB_TOKEN;
                    scanpos++;
                }
                break;
            case '/':
                if (scanpos[1] == '/')
                    scantoken = EOL_TOKEN;
                else
                {
                    scantoken = DIV_TOKEN;
                    scanpos++;
                }
                break;
            case ':':
                if (scanpos[1] == ':')
                {
                    scantoken = TRIELSE_TOKEN;
                    scanpos += 2;
                }
                else
                {
                    scantoken = COLON_TOKEN;
                    scanpos++;
                }
                break;
            case '?':
                if (scanpos[1] == '?')
                {
                    scantoken = TERNARY_TOKEN;
                    scanpos += 2;
                }
                break;
            default:
                /*
                 * Simple single character tokens.
                 */
                scantoken = TOKEN(*scanpos++);
        }
    }
    tokenlen = scanpos - tokenstr;
    return (scantoken);
}
void scan_rewind(char *backptr)
{
    scanpos = tokenstr = backptr;
}
int scan_lookahead(void)
{
    char *backscan = scanpos;
    char *backtkn  = tokenstr;
    char *backstr  = strpos;
    int prevtoken  = scantoken;
    int prevlen    = tokenlen;
    int look       = scan();
    scanpos        = backscan;
    tokenstr       = backtkn;
    strpos         = backstr;
    scantoken      = prevtoken;
    tokenlen       = prevlen;
    return (look);
}
char inputline[512];
char conststr[1024];
int next_line(void)
{
    int len;
    t_token token;
    char* new_filename;
    strpos = conststr;
    if (inputfile == NULL)
    {
        /*
         * First-time init
         */
        inputfile = stdin;
        filename = "<stdin>";
    }
    if (*scanpos == ';')
    {
        statement = ++scanpos;
        scantoken = EOL_TOKEN;
    }
    else
    {
        if (!(scantoken == EOL_TOKEN || scantoken == EOF_TOKEN))
        {
            fprintf(stderr, "scantoken = %d (%c)\n", scantoken & 0x7F, scantoken & 0x7F);
            parse_error("Extraneous characters");
            return EOF_TOKEN;
        }
        statement = inputline;
        scanpos   = inputline;
        /*
         * Read next line from the current file, and strip newline from the end.
         */
        if (fgets(inputline, 512, inputfile) == NULL)
        {
            inputline[0] = 0;
            /*
             * At end of file, return to previous file if any, else return EOF_TOKEN
             */
            if (outer_inputfile != NULL)
            {
                fclose(inputfile);
                free(filename);
                inputfile = outer_inputfile;
                filename = outer_filename;
                lineno = outer_lineno - 1; // -1 because we're about to incr again
                outer_inputfile = NULL;
            }
            else
            {
                scantoken = EOF_TOKEN;
                return EOF_TOKEN;
            }
        }
        len = strlen(inputline);
        if (len > 0 && inputline[len-1] == '\n')
            inputline[len-1] = '\0';
        lineno++;
        scantoken = EOL_TOKEN;
        printf("; %s: %04d: %s\n", filename, lineno, inputline);
    }
    token = scan();
    /*
     * Handle single level of file inclusion
     */
    if (token == INCLUDE_TOKEN)
    {
        token = scan();
        if (token != STRING_TOKEN)
        {
            parse_error("Missing include filename");
            scantoken = EOF_TOKEN;
            return EOF_TOKEN;
        }
        if (outer_inputfile != NULL)
        {
            parse_error("Only one level of includes allowed");
            scantoken = EOF_TOKEN;
            return EOF_TOKEN;
        }
        if (scan() != EOL_TOKEN)
        {
            parse_error("Extraneous characters");
        }
        outer_inputfile = inputfile;
        outer_filename = filename;
        outer_lineno = lineno;
        new_filename = (char *) malloc(*((unsigned char *)constval) + 1);
        strncpy(new_filename, (char *)(constval + 1), *((unsigned char *)constval) + 1);
        inputfile = fopen(new_filename, "r");
        if (inputfile == NULL)
        {
            parse_error("Error opening include file");
            scantoken = EOF_TOKEN;
            return EOF_TOKEN;
        }
        filename = new_filename;
        lineno = 0;
        return next_line();
    }
    return token;
}
int parse_vars(int type)
{
    long value;
    int idlen, size, cfnparms, emit = 0;
    long cfnvals;
    char *idstr;

    switch (scantoken)
    {
        case SYSFLAGS_TOKEN:
            if (type & (EXTERN_TYPE | LOCAL_TYPE))
                parse_error("SYSFLAGS must be global");
            if (!parse_constexpr(&value, &size))
                parse_error("Bad constant");
            emit_sysflags(value);
            break;
        case EXPORT_TOKEN:
            if (type & (EXTERN_TYPE | LOCAL_TYPE))
                parse_error("Cannot export local/imported variables");
            type  = EXPORT_TYPE;
            idstr = tokenstr;
            if (scan() != BYTE_TOKEN && scantoken != WORD_TOKEN)
            {
                /*
                 * This could be an exported definition.
                 */
                scan_rewind(idstr);
                scan();
                return (0);
            }
            break;
        case PREDEF_TOKEN:
            /*
             * Pre definition.
             */
            do
            {
                if (scan() == ID_TOKEN)
                {
                    type  = (type & ~FUNC_PARMVALS) | PREDEF_TYPE;
                    idstr = tokenstr;
                    idlen = tokenlen;
                    cfnparms = 0;
                    cfnvals  = 1; // Default to one return value for compatibility
                    if (scan() == OPEN_PAREN_TOKEN)
                    {
                        do
                        {
                            if (scan() == ID_TOKEN)
                            {
                                cfnparms++;
                                scan();
                            }
                        } while (scantoken == COMMA_TOKEN);
                        if (scantoken != CLOSE_PAREN_TOKEN)
                            parse_error("Bad function parameter list");
                        scan();
                    }
                    if (scantoken == POUND_TOKEN)
                    {
                        if (!parse_const(&cfnvals))
                            parse_error("Invalid def return value count");
                        scan();
                    }
                    type |= funcparms_type(cfnparms) | funcvals_type(cfnvals);
                    idfunc_add(idstr, idlen, type, tag_new(type));
                }
                else
                    parse_error("Bad function pre-declaration");
            } while (scantoken == COMMA_TOKEN);
            break;
        case IMPORT_TOKEN:
            if (emit || type != GLOBAL_TYPE)
                parse_error("IMPORT after emitting data");
            parse_mods();
            break;
        case EOL_TOKEN:
            break;
        default:
            return (0);
    }
    return (1);
}
int parse_mods(void)
{
    if (scantoken == IMPORT_TOKEN)
    {
        if (scan() != ID_TOKEN)
            parse_error("Bad import definition");
        emit_moddep(tokenstr, tokenlen);
        scan();
        while (parse_vars(EXTERN_TYPE)) next_line();
        if (scantoken != END_TOKEN)
            parse_error("Missing END");
        scan();
    }
    if (scantoken == EOL_TOKEN)
        return (1);
    emit_moddep(0, 0);
    return (0);
}
int parse_defs(void)
{
    char c, *idstr;
    int idlen, func_tag, cfnparms, cfnvals, type = GLOBAL_TYPE, pretype;
    static char bytecode = 0;

    switch (scantoken)
    {
        case CONST_TOKEN:
        case STRUC_TOKEN:
            return parse_vars(GLOBAL_TYPE);
        case EXPORT_TOKEN:
            if (scan() != DEF_TOKEN && scantoken != ASM_TOKEN)
                parse_error("Bad export definition");
            type = EXPORT_TYPE;
    }
    if (scantoken == DEF_TOKEN)
    {
        if (scan() != ID_TOKEN)
            parse_error("Missing function name");
        emit_bytecode_seg();
        lambda_cnt  = 0;
        bytecode    = 1;
        cfnparms    = 0;
        infuncvals  = 1; // Defaut to one return value for compatibility
        infunc      = 1;
        type       |= DEF_TYPE;
        idstr       = tokenstr;
        idlen       = tokenlen;
        idlocal_reset();
        /*
         * Parse parameters and return value count
         */
        if (scan() == OPEN_PAREN_TOKEN)
        {
            do
            {
                if (scan() == ID_TOKEN)
                {
                    cfnparms++;
                    idlocal_add(tokenstr, tokenlen, WORD_TYPE, 2);
                    scan();
                }
            } while (scantoken == COMMA_TOKEN);
            if (scantoken != CLOSE_PAREN_TOKEN)
                parse_error("Bad function parameter list");
            scan();
        }
        if (scantoken == POUND_TOKEN)
        {
            if (!parse_const(&infuncvals))
                parse_error("Invalid def return value count");
            scan();
        }
        type |= funcparms_type(cfnparms) | funcvals_type(infuncvals);
        if (idglobal_lookup(idstr, idlen) >= 0)
        {
            pretype = id_type(idstr, idlen);
            if (!(pretype & PREDEF_TYPE))
                parse_error("Mismatch function type");
            if ((pretype & FUNC_PARMVALS) != (type & FUNC_PARMVALS))
                parse_error("Mismatch function params/return values");
            emit_idfunc(id_tag(idstr, idlen), PREDEF_TYPE, idstr, 0);
            func_tag = tag_new(type);
            idfunc_set(idstr, idlen, type, func_tag); // Override any predef type & tag
        }
        else
        {
            func_tag = tag_new(type);
            idfunc_add(idstr, idlen, type, func_tag);
        }
        c = idstr[idlen];
        idstr[idlen] = '\0';
        emit_idfunc(func_tag, type, idstr, 1);
        idstr[idlen] = c;
        /*
         * Parse local vars
         */
        while (parse_vars(LOCAL_TYPE)) next_line();
        emit_enter(cfnparms);
        prevstmnt = 0;
        while (parse_stmnt()) next_line();
        infunc = 0;
        if (scantoken != END_TOKEN)
            parse_error("Missing END");
        scan();
        if (prevstmnt != RETURN_TOKEN)
        {
            if (infuncvals)
               parse_warn("No return values");
            for (cfnvals = 0; cfnvals < infuncvals; cfnvals++)
                emit_const(0);
            emit_leave();
        }
        for (cfnvals = 0; cfnvals < lambda_cnt; cfnvals++)
            emit_lambdafunc(lambda_tag[cfnvals], lambda_id[cfnvals], lambda_cparams[cfnvals], lambda_seq[cfnvals]);
        lambda_cnt = 0;
        return (1);
    }
    else if (scantoken == ASM_TOKEN)
    {
        if (scan() != ID_TOKEN)
            parse_error("Missing function name");
        if (bytecode)
            parse_error("ASM code only allowed before DEF code");
        cfnparms    = 0;
        infuncvals  = 1; // Defaut to one return value for compatibility
        infunc      = 1;
        type       |= ASM_TYPE;
        idstr       = tokenstr;
        idlen       = tokenlen;
        idlocal_reset();
        if (scan() == OPEN_PAREN_TOKEN)
        {
            do
            {
                if (scan() == ID_TOKEN)
                {
                    cfnparms++;
                    scan();
                }
            }
            while (scantoken == COMMA_TOKEN);
            if (scantoken != CLOSE_PAREN_TOKEN)
                parse_error("Bad function parameter list");
            scan();
        }
        if (scantoken == POUND_TOKEN)
        {
            if (!parse_const(&infuncvals))
                parse_error("Invalid def return value count");
            scan();
        }
        type |= funcparms_type(cfnparms) | funcvals_type(infuncvals);
        if (idglobal_lookup(idstr, idlen) >= 0)
        {
            pretype = id_type(idstr, idlen);
            if (!(pretype & PREDEF_TYPE))
                parse_error("Mismatch function type");
            if ((pretype & FUNC_PARMVALS) != (type & FUNC_PARMVALS))
                parse_error("Mismatch function params/return values");
            emit_idfunc(id_tag(idstr, idlen), PREDEF_TYPE, idstr, 0);
            func_tag = tag_new(type);
            idfunc_set(idstr, idlen, type, func_tag); // Override any predef type & tag
        }
        else
        {
            func_tag = tag_new(type);
            idfunc_add(idstr, idlen, type, func_tag);
        }
        c = idstr[idlen];
        idstr[idlen] = '\0';
        emit_idfunc(func_tag, type, idstr, 0);
        idstr[idlen] = c;
        do
        {
            if (scantoken != END_TOKEN && scantoken != EOL_TOKEN)
            {
                scantoken = EOL_TOKEN;
                emit_asm(inputline);
            }
            next_line();
        } while (scantoken != END_TOKEN);
        scan();
        infunc = 0;
        return (1);
    }
    return (scantoken == EOL_TOKEN);
}
int parse_forth(void)
{
    emit_header();
    if (next_line())
    {
        while (parse_mods())            next_line();
        while (parse_vars(GLOBAL_TYPE)) next_line();
        while (parse_defs())            next_line();
        emit_bytecode_seg();
        emit_start();
        idlocal_reset();
        emit_idfunc(0, 0, NULL, 1);
        prevstmnt = 0;
        if (scantoken != DONE_TOKEN && scantoken != EOF_TOKEN)
        {
            while (parse_stmnt()) next_line();
            if (scantoken != DONE_TOKEN)
                parse_error("Missing DONE");
        }
        if (prevstmnt != RETURN_TOKEN)
        {
            emit_const(0);
            emit_ret();
        }
    }
    emit_trailer();
    return (0);
}

int outflags = 0;
int main(int argc, char **argv)
{
    int j, i, flags = 0;
    for (i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            j = 1;
            while (argv[i][j])
            {
                switch(argv[i][j++])
                {
                    case 'A':
                        outflags |= ACME;
                        break;
                    case 'M':
                        outflags |= MODULE;
                        break;
                    case 'O':
                        outflags |= OPTIMIZE;
                        break;
                    case 'N':
                        outflags |= NO_COMBINE;
                        break;
                    case 'W':
                        outflags |= WARNINGS;
                }
            }
        }
    }
    emit_flags(outflags);
    init_dict();
    if (parse_forth())
    {
        fprintf(stderr, "Compilation complete.\n");
    }
    return (0);
}
