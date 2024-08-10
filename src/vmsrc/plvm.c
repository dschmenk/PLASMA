#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include "plvm.h"

/*
 * Debug flag
 */
int show_state = 0;
/*
 * VM def routines
 */
#define MAX_DEF_CNT    1024
code * vm_def[MAX_DEF_CNT];
int def_count = 0;
/*
 * VM callouts
 */
#define MAX_NATV_CNT    1024
VM_Callout vm_natv[MAX_NATV_CNT];
int natv_count = 0;
/*
 * Memory for 6502 and VM
 */
byte mem_6502[MEM_SIZE];
word eval_stack[ESTK_SIZE];
word *esp = &eval_stack[ESTK_SIZE];
/*
 * BRK and VM entrypoint hooks
 */
int vm_irq(M6502 *mpu, uword address, byte data)
{
    uword addr, handle;

    if (mem_6502[mpu->registers->s + 0x101] & flagB)
    {
        //
        // Handle BRK instructions
        //
        PLP; // restore status reg
        addr  = mem_6502[++mpu->registers->s + 0x100];
        addr |= mem_6502[++mpu->registers->s + 0x100] << 8;
        //switch (mem_6502[addr]) // BRK type
        //{
        //    default:
        //        fprintf(stderr, "Unkonw BRK value!\n");
        //}
        fprintf(stderr, "BRK: $%04X\r\n", addr);
        exit (-1);
    }
    fprintf(stderr, "Unkonw IRQ!\n");
    RTI;
}
//
// Inline byte code
//
int vm_indef(M6502 *mpu, uword address, byte data)
{
    uword addr;

    addr  = mem_6502[++mpu->registers->s + 0x100];
    addr |= mem_6502[++mpu->registers->s + 0x100] << 8;
    vm_interp(mpu, &mem_6502[addr + 1]);
    RTS;
}
int vm_iidef(M6502 *mpu, uword address, byte data)
{
    uword addr;

    addr  = mem_6502[++mpu->registers->s + 0x100];
    addr |= mem_6502[++mpu->registers->s + 0x100] << 8;
    vm_interp(mpu, mem_6502 + UWORD_PTR(&mem_6502[addr + 1]));
    RTS;
}
//
// Extended memory byte code
//
int vm_exdef(M6502 *mpu, uword address, byte data)
{
    uword addr;

    addr  = mem_6502[++mpu->registers->s + 0x100];
    addr |= mem_6502[++mpu->registers->s + 0x100] << 8;
    vm_interp(mpu, vm_def[UWORD_PTR(&mem_6502[addr + 1])]);
    RTS;
}
//
// Native code
//
int vm_natvdef(M6502 *mpu, uword address, byte data)
{
    uword addr;

    addr  = mem_6502[++mpu->registers->s + 0x100];
    addr |= mem_6502[++mpu->registers->s + 0x100] << 8;
    vm_natv[mem_6502[addr + 1]](mpu);
    RTS;
}
int vm_adddef(code * defaddr)
{
    vm_def[def_count++] = defaddr;
    return def_count;
}
int vm_addnatv(VM_Callout vm_callout)
{
    vm_natv[natv_count++] = vm_callout;
    return natv_count;
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
        DW NATV,JUMPZ,JUMP                                         ; C0 C2 C4
*/
#define internalize()                                               \
    vm_fp = UWORD_PTR(&mem_6502[FP]);                               \
    vm_pp = UWORD_PTR(&mem_6502[PP]);                               \
    esp   = &eval_stack[ESTK_SIZE];                                 \
    for (val = ESTK_SIZE - 1; val >= mpu->registers->x; val--)      \
        PUSH(mem_6502[ESTKL + val] | (mem_6502[ESTKH + val] << 8))
#define externalize()                                               \
    mem_6502[FPL] = vm_fp;                                          \
    mem_6502[FPH] = vm_fp >> 8;                                     \
    mem_6502[PPL] = vm_pp;                                          \
    mem_6502[PPH] = vm_pp >> 8;                                     \
    ea = ESTK_SIZE-1;                                               \
    for (vm_sp = &eval_stack[ESTK_SIZE-1]; vm_sp >= esp; vm_sp--) { \
        mem_6502[ESTKL + ea] = *vm_sp;                              \
        mem_6502[ESTKH + ea] = *vm_sp >> 8;                         \
        ea--;                                                       \
    }                                                               \
    mpu->registers->x = ea + 1

void vm_interp(M6502 *mpu, code *vm_ip)
{
    int val, ea, frmsz, parmcnt;
    uword vm_fp, vm_pp;
    word *vm_sp;

    internalize();
    while (1)
    {
        if ((esp - eval_stack) < 0 || (esp - eval_stack) > ESTK_SIZE)
        {
            printf("Eval stack over/underflow! - $%04X: $%02X [%d]\r\n", (unsigned int)(vm_ip - mem_6502), (unsigned int)*vm_ip, (int)(ESTK_SIZE - (esp - eval_stack)));
            show_state = 1;
        }
        if (show_state)
        {
            char cmdline[16];
            word *dsp = &eval_stack[ESTK_SIZE - 1];
            printf("$%04X: $%02X [ ", (unsigned int)(vm_ip - mem_6502), (unsigned int)*vm_ip);
            while (dsp >= esp)
                printf("$%04X ", (unsigned int)((*dsp--) & 0xFFFF));
            printf("]\r\n");
            fgets(cmdline, 15, stdin);
        }
        switch (*vm_ip++)
        {
            /*
             * 0x00-0x1F CONSTANT
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
                PUSH(*(vm_ip - 1)/2);
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
                    vm_ip += WORD_PTR(vm_ip) ;
                else
                    vm_ip += 2;
                break;
            case 0x24: // BRNE
                val = POP;
                if (val != POP)
                    vm_ip += WORD_PTR(vm_ip) ;
                else
                    vm_ip += 2;
                break;
            case 0x26: // LA : TOS = @VAR ; equivalent to CW ADDRESSOF(VAR)
                PUSH(WORD_PTR(vm_ip));
                vm_ip += 2;
                break;
            case 0x28: // LLA : TOS = @LOCALVAR ; equivalent to CW FRAMEPTR+OFFSET(LOCALVAR)
                PUSH(vm_fp + BYTE_PTR(vm_ip));
                vm_ip++;
                break;
            case 0x2A: // CB : TOS = CONSTANTBYTE (IP)
                PUSH(BYTE_PTR(vm_ip));
                vm_ip++;
                break;
            case 0x2C: // CW : TOS = CONSTANTWORD (IP)
                PUSH(WORD_PTR(vm_ip));
                vm_ip += 2;
                break;
            case 0x2E: // CS: TOS = CONSTANTSTRING (IP)
                PUSH(vm_ip - mem_6502);
                vm_ip += BYTE_PTR(vm_ip) + 1;
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
                val = POP + BYTE_PTR(vm_ip);
                PUSH(val);
                vm_ip++;
                break;
            case 0x3A: // SUBI
                val = POP - BYTE_PTR(vm_ip);
                PUSH(val);
                vm_ip++;
                break;
            case 0x3C: // ANDI
                val = POP & BYTE_PTR(vm_ip);
                PUSH(val);
                vm_ip++;
                break;
            case 0x3E: // ORI
                val = POP | BYTE_PTR(vm_ip);
                PUSH(val);
                vm_ip++;
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
                    vm_ip += WORD_PTR(vm_ip) ;
                else
                    vm_ip += 2;
                break;
            case 0x4E: // BRTRU : TOS ? IP += (IP)
                if (POP)
                    vm_ip += WORD_PTR(vm_ip);
                else
                    vm_ip += 2;
                break;
                /*
                 * 0x50-0x5F
                 */
            case 0x50: // BRNCH : IP += (IP)
                vm_ip += WORD_PTR(vm_ip);
                break;
            case 0x52: // SELECT
                val = POP;
                vm_ip += WORD_PTR(vm_ip);
                parmcnt = BYTE_PTR(vm_ip);
                vm_ip++;
                while (parmcnt--)
                {
                    if (WORD_PTR(vm_ip) == val)
                    {
                        vm_ip += 2;
                        vm_ip += WORD_PTR(vm_ip);
                        break;
                    }
                    else
                        vm_ip += 4;
                }
                break;
            case 0x54: // CALL : TOFP = IP, IP = (IP) ; call
                mpu->registers->pc = UWORD_PTR(vm_ip);
                mem_6502[0x00FF] = 0xFF; // RTN instruction
                mem_6502[mpu->registers->s-- + 0x100] = 0x00; // Address of $FF (RTN) instruction
                mem_6502[mpu->registers->s-- + 0x100] = 0xFE;
                //mpu->flags |= M6502_SingleStep;
                externalize();
                if (show_state)
                    printf("CALL: $%04X\r\n", mpu->registers->pc);
                M6502_exec(mpu);
                internalize();
                vm_ip += 2;
                break;
            case 0x56: // ICALL : IP = TOS ; indirect call
                mpu->registers->pc = UPOP;
                mem_6502[0x00FF] = 0xFF; // RTN instruction
                mem_6502[mpu->registers->s-- + 0x100] = 0x00; // Address of $FF (RTN) instruction
                mem_6502[mpu->registers->s-- + 0x100] = 0xFE;
                externalize();
                if (show_state)
                    printf("ICAL: $%04X\r\n", mpu->registers->pc);
                M6502_exec(mpu);
                internalize();
                break;
            case 0x58: // ENTER : NEW FRAME, FOREACH PARAM LOCALVAR = TOS
                frmsz = BYTE_PTR(vm_ip);
                vm_ip++;
                if (show_state)
                    printf("< $%04X: $%04X > ", vm_fp - frmsz, vm_fp);
                vm_fp -= frmsz;
                parmcnt = BYTE_PTR(vm_ip);
                vm_ip++;
                while (parmcnt--)
                {
                    val = POP;
                    mem_6502[vm_fp + parmcnt * 2 + 0] = val;
                    mem_6502[vm_fp + parmcnt * 2 + 1] = val >> 8;
                    if (show_state)
                        printf("< $%04X: $%04X > ", vm_fp + parmcnt * 2 + 0, mem_6502[vm_fp + parmcnt * 2 + 0] | (mem_6502[vm_fp + parmcnt * 2 + 1] >> 8));
                }
                if (show_state)
                    printf("\n");
                break;
            case 0x5A: // LEAVE : DEL FRAME, IP = TOFP
                vm_fp += BYTE_PTR(vm_ip);
            case 0x5C: // RET : IP = TOFP
                externalize();
                return;
            case 0x5E: // CFFB : TOS = CONSTANTBYTE(IP) | 0xFF00
                PUSH(BYTE_PTR(vm_ip) | 0xFF00);
                vm_ip++;
                break;
                /*
                 * 0x60-0x6F
                 */
            case 0x60: // LB : TOS = BYTE (TOS)
                ea = (uword)(POP);
                PUSH(mem_6502[ea]);
                break;
            case 0x62: // LW : TOS = WORD (TOS)
                ea = UPOP;
                PUSH(mem_6502[ea] | (mem_6502[ea + 1] << 8));
                break;
            case 0x64: // LLB : TOS = LOCALBYTE [IP]
                PUSH(mem_6502[(uword)(vm_fp + BYTE_PTR(vm_ip))]);
                vm_ip++;
                break;
            case 0x66: // LLW : TOS = LOCALWORD [IP]
                ea = (uword)(vm_fp + BYTE_PTR(vm_ip));
                PUSH(mem_6502[ea] | (mem_6502[ea + 1] << 8));
                vm_ip++;
                break;
            case 0x68: // LAB : TOS = BYTE (IP)
                PUSH(mem_6502[UWORD_PTR(vm_ip)]);
                vm_ip += 2;
                break;
            case 0x6A: // LAW : TOS = WORD (IP)
                ea = UWORD_PTR(vm_ip);
                PUSH(mem_6502[ea] | (mem_6502[ea + 1] << 8));
                vm_ip += 2;
                break;
            case 0x6C: // DLB : TOS = TOS, LOCALBYTE [IP] = TOS
                mem_6502[(uword)(vm_fp + BYTE_PTR(vm_ip))] = TOS;
                TOS = TOS & 0xFF;
                vm_ip++;
                break;
            case 0x6E: // DLW : TOS = TOS, LOCALWORD [IP] = TOS
                ea = (uword)(vm_fp + BYTE_PTR(vm_ip));
                mem_6502[ea]     = TOS;
                mem_6502[ea + 1] = TOS >> 8;
                vm_ip++;
                break;
                /*
                 * 0x70-0x7F
                 */
            case 0x70: // SB : BYTE (TOS-1) = TOS
                ea  = UPOP;
                val = POP;
                mem_6502[ea] = val;
                break;
            case 0x72: // SW : WORD (TOS-1) = TOS
                ea  = UPOP;
                val = POP;
                mem_6502[ea]     = val;
                mem_6502[ea + 1] = val >> 8;
                break;
            case 0x74: // SLB : LOCALBYTE [TOS] = TOS-1
                mem_6502[(uword)(vm_fp + BYTE_PTR(vm_ip))] = POP;
                vm_ip++;
                break;
            case 0x76: // SLW : LOCALWORD [TOS] = TOS-1
                ea  = (uword)(vm_fp + BYTE_PTR(vm_ip));
                val = POP;
                mem_6502[ea]     = val;
                mem_6502[ea + 1] = val >> 8;
                vm_ip++;
                break;
            case 0x78: // SAB : BYTE (IP) = TOS
                mem_6502[UWORD_PTR(vm_ip)] = POP;
                vm_ip += 2;
                break;
            case 0x7A: // SAW : WORD (IP) = TOS
                ea = UWORD_PTR(vm_ip);
                val = POP;
                mem_6502[ea]     = val;
                mem_6502[ea + 1] = val >> 8;
                vm_ip += 2;
                break;
            case 0x7C: // DAB : TOS = TOS, BYTE (IP) = TOS
                mem_6502[UWORD_PTR(vm_ip)] = TOS;
                TOS = TOS & 0xFF;
                vm_ip += 2;
                break;
            case 0x7E: // DAW : TOS = TOS, WORD (IP) = TOS
                ea = UWORD_PTR(vm_ip);
                mem_6502[ea]     = TOS;
                mem_6502[ea + 1] = TOS >> 8;
                vm_ip += 2;
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
                    vm_ip += WORD_PTR(vm_ip);
                else
                    vm_ip += 2;
                PUSH(val);
                break;
            case 0xA2: // BRLT : TOS-1 < TOS ? IP += (IP)
                val = POP;
                if (TOS > val)
                    vm_ip += WORD_PTR(vm_ip);
                else
                    vm_ip += 2;
                PUSH(val);
                break;
            case 0xA4: // INCBRLE : TOS = TOS + 1
                val = POP;
                val++;
                if (TOS >= val)
                    vm_ip += WORD_PTR(vm_ip);
                else
                    vm_ip += 2;
                PUSH(val);
                break;
            case 0xA6: // ADDBRLE : TOS = TOS + TOS-1
                val = POP;
                ea  = POP;
                val = ea + val;
                if (TOS >= val)
                    vm_ip += WORD_PTR(vm_ip);
                else
                    vm_ip += 2;
                PUSH(val);
                break;
            case 0xA8: // DECBRGE : TOS = TOS - 1
                val = POP;
                val--;
                if (TOS <= val)
                    vm_ip += WORD_PTR(vm_ip);
                else
                    vm_ip += 2;
                PUSH(val);
                break;
            case 0xAA: // SUBBRGE : TOS = TOS-1 - TOS
                val = POP;
                ea  = POP;
                val = ea - val;
                if (TOS <= val)
                    vm_ip += WORD_PTR(vm_ip);
                else
                    vm_ip += 2;
                PUSH(val);
                break;
            case 0xAC: // BRAND : SHORT CIRCUIT AND
                if (TOS) // EVALUATE RIGHT HAND OF AND
                {
                    POP;
                    vm_ip += 2;
                }
                else // MUST BE FALSE, SKIP RIGHT HAND
                {
                    vm_ip += WORD_PTR(vm_ip);
                }
                break;
            case 0xAE: // BROR : SHORT CIRCUIT OR
                if (!TOS) // EVALUATE RIGHT HAND OF OR
                {
                    POP;
                    vm_ip += 2;
                }
                else // MUST BE TRUE, SKIP RIGHT HAND
                {
                    vm_ip += WORD_PTR(vm_ip);
                }
                break;
                /*
                 * 0xB0-0xBF
                 */
            case 0xB0: // ADDLB : TOS = TOS + LOCALBYTE[vm_ip]
                val = POP + mem_6502[(uword)(vm_fp + BYTE_PTR(vm_ip))];
                PUSH(val);
                vm_ip++;
                break;
            case 0xB2: // ADDLW : TOS = TOS + LOCALWORD[vm_ip]
                ea  = (uword)(vm_fp + BYTE_PTR(vm_ip));
                val = POP + (mem_6502[ea] | (mem_6502[ea + 1] << 8));
                PUSH(val);
                vm_ip++;
                break;
            case 0xB4: // ADDAB : TOS = TOS + BYTE[vm_ip]
                val = POP + mem_6502[UWORD_PTR(vm_ip)];
                PUSH(val);
                vm_ip  += 2;
                break;
            case 0xB6: // ADDAW : TOS = TOS + WORD[vm_ip]
                ea  = UWORD_PTR(vm_ip);
                val = POP + (mem_6502[ea] | (mem_6502[ea + 1] << 8));
                PUSH(val);
                vm_ip  += 2;
                break;
            case 0xB8: // IDXLB : TOS = TOS + LOCALBYTE[vm_ip]*2
                val = POP + mem_6502[(uword)(vm_fp + BYTE_PTR(vm_ip))] * 2;
                PUSH(val);
                vm_ip++;
                break;
            case 0xBA: // IDXLW : TOS = TOS + LOCALWORD[vm_ip]*2
                ea  = (uword)(vm_fp + BYTE_PTR(vm_ip));
                val = POP + (mem_6502[ea] | (mem_6502[ea + 1] << 8)) * 2;
                PUSH(val);
                vm_ip++;
                break;
            case 0xBC: // IDXAB : TOS = TOS + BYTE[vm_ip]*2
                val = POP + mem_6502[UWORD_PTR(vm_ip)] * 2;
                PUSH(val);
                vm_ip  += 2;
                break;
            case 0xBE: // IDXAW : TOS = TOS + WORD[vm_ip]*2
                ea  = UWORD_PTR(vm_ip);
                val = POP + (mem_6502[ea] | (mem_6502[ea + 1] << 8)) * 2;
                PUSH(val);
                vm_ip  += 2;
                break;
            case 0xC2: // JUMPZ - compiled Forth
                if (!POP)
                    vm_ip = &mem_6502[UWORD_PTR(vm_ip)];
                else
                    vm_ip += 2;
                break;
            case 0xC4: // JUMP - compiled Forth
                vm_ip = &mem_6502[UWORD_PTR(vm_ip)];
                break;
                /*
                 * Odd codes and everything else are errors.
                 */
            default:
                fprintf(stderr, "Illegal opcode 0x%02X @ 0x%04X\n", (unsigned int)vm_ip[-1], (unsigned int)(vm_ip - mem_6502));
                exit(-1);
        }
    }
}
