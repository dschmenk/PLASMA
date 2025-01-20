#include "lib6502/config.h"
#include "lib6502/lib6502.h"

typedef uint8_t  code;
typedef uint8_t  byte;
typedef int16_t  word;
typedef uint16_t uword;
typedef uint16_t address;

/*
 * Bytecode memory
 */
#ifdef __LITTLE_ENDIAN__
#define BYTE_PTR(bp)    (*(byte *)(bp))
#define WORD_PTR(wp)    (*(word *)(wp))
#define UWORD_PTR(wp)   (*(uword *)(wp))
#else
#define BYTE_PTR(bp)    ((byte)((bp)[0]))
#define WORD_PTR(bp)    ((word)((bp)[0]  | ((bp)[1] << 8)))
#define UWORD_PTR(bp)   ((uword)((bp)[0] | ((bp)[1] << 8)))
#endif
/*
 * 6502 H/W stack
 */
#define PHA             (mem_6502[0x0100 + mpu->registers->s--]=mpu->registers->a)
#define PLA             (mpu->registers->a=mem_6502[++mpu->registers->s + 0x0100])
#define PHP             (mem_6502[0x0100 + mpu->registers->s--]=mpu->registers->p)
#define PLP             (mpu->registers->p=mem_6502[++mpu->registers->s + 0x0100])
#define RTI                                                 \
    {                                                       \
        uword pc;                                           \
        PLP;                                                \
        pc  = mem_6502[++mpu->registers->s + 0x100];        \
        pc |= mem_6502[++mpu->registers->s + 0x100]<<8;     \
        return pc;                                          \
    }
#define RTS                                                 \
    {                                                       \
        uword pc;                                           \
        pc  = mem_6502[++mpu->registers->s + 0x100];        \
        if (!mpu->registers->s) pfail("RTS: SP LSB underflow");      \
        pc |= mem_6502[++mpu->registers->s + 0x100]<<8;     \
        if (!mpu->registers->s) pfail("RTS: SP MSB underflow");      \
        return pc + 1;                                      \
    }
/*
 * 6502 VM eval stack
 */
#define PUSH_ESTK(v)                                        \
    {                                                       \
        --mpu->registers->x;                                \
        mem_6502[ESTKL+mpu->registers->x] = (byte)(v);      \
        mem_6502[ESTKH+mpu->registers->x] = (byte)((v)>>8); \
    }
#define PULL_ESTK(v)                                        \
    {                                                       \
        (v) = mem_6502[ESTKL+mpu->registers->x]             \
            | mem_6502[ESTKH+mpu->registers->x]<<8;         \
        ++mpu->registers->x;                                \
    }
/*
 * 6502 memory map
 */
#define MEM6502_SIZE    0x00010000
#define ESTK_DEPTH      32
#define CMDLINE_STR     0x01FF
#define CMDLINE_BUF     0x0200
#define SYSPATH_STR     0x0280
#define SYSPATH_BUF     0x0281
/*
 * Zero page VM locations matching Apple ZP
 */
#define ESTKH           0xA0 // Hi bytes of VM stack
#define ESTKL           0xC0 // Lo bytes of VM stack
#define FP              0xE0 // Frame pointer
#define FPL             0xE0
#define FPH             0xE1
#define PP              0xE2 // String pool pointer
#define PPL             0xE2
#define PPH             0xE3
#define ESP             0xE5 // Temp storage for VM stack pointer
/*
 * VM entrypoints
 */
#define VM_NATV_DEF         3
#define VM_EXT_DEF          2
#define VM_DEF              1
#define VM_INLINE_DEF       0
#define VM_SYSCALL          0xFF10
#define VM_NATV_ENTRY       0xFF0E
#define VM_EXT_ENTRY        0xFF0C
#define VM_INDIRECT_ENTRY   0xFF08
#define VM_INLINE_ENTRY     0xFF04
#define VM_IRQ_ENTRY        0xFF00
/*
 * Allocate external code memory
 */
#define code_alloc(size)    malloc(size)
#define TRACE               1
#define SINGLE_STEP         2
/*
 * Fatal error
 */
extern void pfail(const char *msg);
/*
 * Key handling
 */
extern int trace;
extern int paused;
extern byte keyqueue;
byte keypressed(M6502 *mpu);
byte keyin(M6502 *mpu);
/*
 * VM callouts
 */
extern void M6502_exec(M6502 *mpu);
typedef void (*VM_Callout)(M6502 *mpu);
extern byte mem_6502[];
extern byte mem_PLVM[];
extern byte *perr;
extern uword cmdsys;
extern int vm_addxdef(code * defaddr);
extern int vm_addnatv(VM_Callout);
extern int vm_irq(M6502 *mpu, uword address, byte data);
extern int vm_indef(M6502 *mpu, uword address, byte data);
extern int vm_iidef(M6502 *mpu, uword address, byte data);
extern int vm_exdef(M6502 *mpu, uword address, byte data);
extern int vm_natvdef(M6502 *mpu, uword address, byte data);
extern void vm_interp(M6502 *mpu, code *vm_ip);
/*
 * Heap routines
 */
uword alloc_heap(uword size);
/*
 * Symbol export routines
 */
extern int stodci(char *str, byte *dci);
extern uword add_sym(byte *dci, uword addr);
extern uword add_natv( VM_Callout natvfn);
extern uword export_natv(char *symstr, VM_Callout natvfn);
/*
 * System I/O routines
 */
extern void export_sysio(void);
extern void sysopen(M6502 *mpu);
extern void sysclose(M6502 *mpu);
extern void sysread(M6502 *mpu);
extern void syswrite(M6502 *mpu);
/*
 * Apple 1 config
 */
int initApple1(M6502 *mpu, uword address, const char *path);

