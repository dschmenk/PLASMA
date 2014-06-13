# PLASMA 123 Internal Architecture
This document describes the low-level implementation of PLASMA. It is not necessary to know how PLASMA is implemented to write PLASMA programs, but understanding how the virtual machine operates can give you insight on how certain operations are carried out and how to write optimal PLASMA code. It *is* a requirement to understand when interfacing to native 6502 code. PLASMA consists of a virtual machine and a compiler to translate PLASMA source code to PLASMA bytecode.

## The Virtual Machine
The 6502 processor is a challenging target for a compiler. Most high level languages do have a compiler avialable targetting the 6502, but none are particularly efficient at code generation. Usually a series of calls into routines that do much of the work, not too dissimlar to a threaded interpreter. Generating inline 6502 leads quickly to code bloat and unwieldy binaries. The trick is to find a happy medium between efficient code execution and small code size. To this end, the PLASMA VM enforces some restrictions that are a result of the 6502 architecture, yet don't hamper the expressiveness of the PLASMA language.

### The Stacks
The PLASMA VM is architected around three stacks: the evaluation stack, the call stack, and the local frame stack. These stacks provide the PLASMA VM with foundation for efficient operation and compact bytecode. The stack architecure also creates a simple target for the PLASMA compiler.

#### The Evaluation Stack
All calculations, data moves, and paramter passing is done on the evaluation stack. This stack is located on the zero page of the 6502; an efficient section of memory that can be addressed with only an eight bit address. As a structure that is accessed more than any other on PLASMA, it makes sense to put it in fastest memory. The evaluation stack is a 16 entry stack that is split into low bytes and high bytes. The 6502's X register is used to index into the evaluation stack. It *always* points to the top of the evaluation stack, so care must be taken to save/restore its value when calling native 6502 code. Parameters and results are also passed on the evaluation stack. Caller and callee must agree on the number of parameters: PLASMA does no error checking. Native functions can pull values from the evaluation stack by using the zero page indexed addressing using the X register.

#### The Call Stack
Function calls use the call stack to save the return address of the calling code. PLASMA uses the 6502 hardware stack for this purpose, as it is the 6502's JSR (Jump SubRoutine) instruction that PLASMA's call opcodes are implemented.

#### The Local Frame Stack
One of the biggest problems to overcome with the 6502 is its very small hardware stack. Algorithms that incorporate recursive procedure calls are very difficult or slow on the 6502. PLASMA takes the middle ground when implementing local frames; a frame pointer on the zero page is indirectly indexed by the Y register. Because the Y register is only eight bits, the local frame size is limited to 256 bytes. 256 bytes really is sufficient for all but the most complex of functions. With a little creative use of dynamic memory allocation, almost anything can be implemented without undue hassle. When a function with parameters is called, the first order of business is to allocate the frame, copy the parameters off the evaluation stack into local variables, and save a link to the previous frame. This is all done automatically with the ENTER opcode. The reverse takes place with the LEAVE opcode when the function exits. Functions that have neither parameters or local variables can forgoe the frame build/destroy process.

### The Bytecodes
The compact code representation comes through the use of opcodes closely matched to the PLASMA compiler. They are:

| OPCODE |                  Description
|:------:|-----------------------------------
| ZERO   | push zero on the stack
| ADD    | add top two values, leave result on top
| SUB    | subtract next from top from top, leave result on top
| MUL    | multiply two topmost stack values, leave result on top
| DIV    | divide next from top by top, leave result on top
| MOD    | divide next from top by top, leave remainder on top
| INCR   | increment top of stack
| DECR   | decrement top of stack
| NEG    | negate top of stack
| COMP   | compliment top of stack
| AND    | bit wise AND top two values, leave result on top
| IOR    | bit wise inclusive OR top two values, leave result on top
| XOR    | bit wise exclusive OR top two values, leave result on top
| LOR    | logical OR top two values, leave result on top
| LAND   | logical AND top two values, leave result on top
| SHL    | shift left next from top by top, leave result on top
| SHR    | shift right next from top by top, leave result on top
| IDXB   | add top of stack to next from top, leave result on top (ADD)
| IDXW   | add 2X top of stack to next from top, leave result on top
| NOT    | logical NOT of top of stack
| LA     | load address
| LLA    | load local address from frame offset
| CB     | constant byte
| CW     | constant word
| SWAP   | swap two topmost stack values
| DROP   | drop top stack value
| DUP    | duplicate top stack value
| PUSH   | push top to call stack
| PULL   | pull from call stack
| BRGT   | branch next from top greater than top
| BRLT   | branch next from top less than top
| BREQ   | branch next from top equal to top
| BRNE   | branch next from top not equal to top
| ISEQ   | if next from top is equal to top, set top true
| ISNE   | if next from top is not equal to top, set top true
| ISGT   | if next from top is greater than top, set top true
| ISLT   | if next from top is less than top, set top true
| ISGE   | if next from top is greater than or equal to top, set top true
| ISLE   | if next from top is less than or equal to top, set top true
| BRFLS  | branch if top of stack is zero
| BRTRU  | branch if top of stack is non-zero
| BRNCH  | branch to address
| CALL   | sub routine call with stack parameters
| ICAL   | sub routine call to indirect address on stack top with stack parameters
| ENTER  | allocate frame size and copy stack parameters to local frame
| LEAVE  | deallocate frame and return from sub routine call
| RET    | return from sub routine call
| LB     | load byte from top of stack address
| LW     | load word from top of stack address
| LLB    | load byte from frame offset
| LLW    | load word from frame offset
| LAB    | load byte from absolute address
| LAW    | load word from absolute address
| SB     | store top of stack byte into next from top address
| SW     | store top of stack word into next from top address
| SLB    | store top of stack into local byte at frame offset
| SLW    | store top of stack into local word at frame offset
| SAB    | store top of stack into byte at absolute address
| SAW    | store top of stack into word at absolute address
| DLB    | duplicate top of stack into local byte at frame offset
| DLW    | duplicate top of stack into local word at frame offset
| DAB    | duplicate top of stack into byte at absolute address
| DAW    | duplicate top of stack into word at absolute address

The opcodes were developed over time by starting with a very basic set of operations and slowly adding opcodes when the PLASMA compiler could improve code density or performance.
