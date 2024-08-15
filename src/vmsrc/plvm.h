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
#define PHA             (mem_6502[mpu->registers->s-- + 0x100]=mpu->registers->a)
#define PLA             (mpu->registers->a=mem_6502[++mpu->registers->s + 0x100])
#define PHP             (mem_6502[mpu->registers->s-- + 0x100]=mpu->registers->p)
#define PLP             (mpu->registers->p=mem_6502[++mpu->registers->s + 0x100])
#define RTI                                                 \
    {                                                       \
        word pc;                                            \
        PLP;                                                \
        pc  = mem_6502[++mpu->registers->s + 0x100];        \
        pc |= mem_6502[++mpu->registers->s + 0x100] << 8;   \
        return pc;                                          \
    }
#define RTS                                                 \
    {                                                       \
        word pc;                                            \
        pc  = mem_6502[++mpu->registers->s + 0x100];        \
        pc |= mem_6502[++mpu->registers->s + 0x100] << 8;   \
        return pc + 1;                                      \
    }
/*
 * 6502 VM eval stack
 */
#define PUSH_ESTK(v)                                        \
    {                                                       \
        --mpu->registers->x;                                \
        mem_6502[ESTKL + mpu->registers->x] = (v);          \
        mem_6502[ESTKH + mpu->registers->x] = (v) >> 8;     \
    }
#define POP_ESTK(v)                                         \
    {                                                       \
        (v) = mem_6502[ESTKL + mpu->registers->x]           \
            | mem_6502[ESTKH + mpu->registers->x] << 8;     \
        ++mpu->registers->x;                                \
    }
/*
 * PLVM eval stack
 */
#define PUSH(v)         (*(--esp))=(v)
#define POP             ((word)(*(esp++)))
#define UPOP            ((uword)(*(esp++)))
#define TOS             (esp[0])
/*
 * 6502 memory map
 */
#define MEM6502_SIZE    0x00010000
#define ESTK_SIZE       16
#define CMDLINE_STR     0x01FF
#define CMDLINE_BUF     0x0200
/*
 * Zero page VM locations matching Apple ZP
 */
#define ESTKH           0xC0 // Hi bytes of VM stack
#define ESTKL           0xD0 // Lo bytes of VM stack
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
#define VM_NATV_ENTRY       0xFFFC
#define VM_EXT_ENTRY        0xFFF8
#define VM_INDIRECT_ENTRY   0xFFF4
#define VM_INLINE_ENTRY     0xFFF0
/*
 * VM callouts
 */
void M6502_exec(M6502 *mpu);
typedef void (*VM_Callout)(M6502 *mpu);
extern int show_state;
extern byte mem_6502[];
extern byte mem_PLVM[];
extern byte *perr;
extern int vm_addxdef(code * defaddr);
extern int vm_addnatv(VM_Callout);
extern int vm_irq(M6502 *mpu, uword address, byte data);
extern int vm_indef(M6502 *mpu, uword address, byte data);
extern int vm_iidef(M6502 *mpu, uword address, byte data);
extern int vm_exdef(M6502 *mpu, uword address, byte data);
extern int vm_natvdef(M6502 *mpu, uword address, byte data);
extern void vm_interp(M6502 *mpu, code *vm_ip);
