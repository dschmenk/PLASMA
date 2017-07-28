typedef struct _opseq {
    int code;
    long val;
    int tag;
    int offsz;
    int type;
    struct _opseq *nextop;
} t_opseq;
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
#define LOGIC_OR_CODE (0x0200|LOGIC_OR_TOKEN)
#define LOGIC_AND_CODE (0x0200|LOGIC_AND_TOKEN)
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
#define PUSH_EXP_CODE 0x031A
#define PULL_EXP_CODE 0x031B
#define BRNCH_CODE  0x031C
#define BRFALSE_CODE 0x031D
#define BRTRUE_CODE 0x031E

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
#define gen_pushexp(seq)    gen_seq(seq,PUSH_EXP_CODE,0,0,0,0)
#define gen_pullexp(seq)    gen_seq(seq,PULL_EXP_CODE,0,0,0,0)
#define gen_drop(seq)       gen_seq(seq,DROP_CODE,0,0,0,0)
#define gen_brfls(seq,tag)  gen_seq(seq,BRFALSE_CODE,0,tag,0,0)
#define gen_brtru(seq,tag)  gen_seq(seq,BRTRUE_CODE,0,tag,0,0)

void emit_flags(int flags);
void emit_header(void);
void emit_trailer(void);
void emit_moddep(char *name, int len);
void emit_sysflags(int val);
void emit_bytecode_seg(void);
void emit_comment(char *s);
void emit_asm(char *s);
void emit_idlocal(char *name, int value);
void emit_idglobal(int value, int size, char *name);
void emit_idfunc(int tag, int type, char *name, int is_bytecode);
void emit_idconst(char *name, int value);
int emit_data(int vartype, int consttype, long constval, int constsize);
void emit_codetag(int tag);
void emit_const(int cval);
void emit_conststr(long conststr);
void emit_lb(void);
void emit_lw(void);
void emit_llb(int index);
void emit_llw(int index);
void emit_lab(int tag, int offset, int type);
void emit_law(int tag, int offset, int type);
void emit_sb(void);
void emit_sw(void);
void emit_slb(int index);
void emit_slw(int index);
void emit_dlb(int index);
void emit_dlw(int index);
void emit_sab(int tag, int offset, int type);
void emit_saw(int tag, int offset, int type);
void emit_dab(int tag, int offset, int type);
void emit_daw(int tag, int offset, int type);
void emit_call(int tag, int type);
void emit_ical(void);
void emit_localaddr(int index);
void emit_globaladdr(int tag, int offset, int type);
void emit_indexbyte(void);
void emit_indexword(void);
int emit_unaryop(t_token op);
int emit_op(t_token op);
void emit_brtru(int tag);
void emit_brfls(int tag);
void emit_brgt(int tag);
void emit_brlt(int tag);
void emit_brne(int tag);
void emit_brnch(int tag);
void emit_empty(void);
void emit_push_exp(void);
void emit_pull_exp(void);
void emit_drop(void);
void emit_dup(void);
void emit_leave(void);
void emit_ret(void);
void emit_enter(int cparams);
void emit_start(void);
void emit_rld(void);
void emit_esd(void);
void release_seq(t_opseq *seq);
int crunch_seq(t_opseq **seq, int pass);
t_opseq *gen_seq(t_opseq *seq, int opcode, long cval, int tag, int offsz, int type);
t_opseq *cat_seq(t_opseq *seq1, t_opseq *seq2);
int emit_seq(t_opseq *seq);
