/*
 * Symbol table types.
 */
#define GLOBAL_TYPE     (0)
#define CONST_TYPE      (1 << 0)
#define WORD_TYPE       (1 << 1)
#define BYTE_TYPE       (1 << 2)
#define VAR_TYPE        (WORD_TYPE | BYTE_TYPE)
#define ASM_TYPE        (1 << 3)
#define DEF_TYPE        (1 << 4)
#define BRANCH_TYPE     (1 << 5)
#define LOCAL_TYPE      (1 << 6)
#define EXTERN_TYPE     (1 << 7)
#define ADDR_TYPE       (VAR_TYPE | FUNC_TYPE | EXTERN_TYPE)
#define WPTR_TYPE       (1 << 8)
#define BPTR_TYPE       (1 << 9)
#define PTR_TYPE        (BPTR_TYPE | WPTR_TYPE)
#define STRING_TYPE     (1 << 10)
#define TAG_TYPE        (1 << 11)
#define EXPORT_TYPE     (1 << 12)
#define PREDEF_TYPE     (1 << 13)
#define FUNC_TYPE       (ASM_TYPE | DEF_TYPE | PREDEF_TYPE)
#define FUNC_PARMS      (0x0F << 16)
#define FUNC_VALS       (0x0F << 20)
#define FUNC_PARMVALS   (FUNC_PARMS|FUNC_VALS)
#define funcparms_type(p) (((p)&0x0F)<<16)
#define funcparms_cnt(t) (((t)>>16)&0x0F)
#define funcvals_type(v) (((v)&0x0F)<<20)
#define funcvals_cnt(t) (((t)>>20)&0x0F)

int id_match(char *name, int len, char *id);
int idlocal_lookup(char *name, int len);
int idglobal_lookup(char *name, int len);
int idconst_lookup(char *name, int len);
int idlocal_add(char *name, int len, int type, int size);
int idglobal_add(char *name, int len, int type, int size);
int id_add(char *name, int len, int type, int size);
void idlocal_reset(void);
void idlocal_save(void);
void idlocal_restore(void);
int idfunc_set(char *name, int len, int type, int tag);
int idfunc_add(char *name, int len, int type, int tag);
int idconst_add(char *name, int len, int value);
int id_tag(char *name, int len);
int id_const(char *name, int len);
int id_type(char *name, int len);
void idglobal_size(int type, int size, int constsize);
int tag_new(int type);
