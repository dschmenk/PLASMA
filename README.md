#PLASMA
##Introduction

PLASMA is a combination of virtual machine and assembler/compiler matched closely to the 6502 architecture.  It is an attempt to satisfy a few challenges surrounding code size, efficient execution, small runtime and fast just-in-time compilation.  By architecting a unique bytecode that maps nearly one-to-one to the higher level representation, the compiler/assembler can be very simple and execute quickly on the Apple II for a self-hosted environment.  A modular approach provides for incremental development and code reuse.  Different projects have led to the architecture of PLASMA, most notably Apple Pascal, FORTH, and my own Java VM for the 6502, VM02. Each has tried to map a generic VM to the 6502 with varying levels of success.  Apple Pascal, based on the USCD Pascal using the p-code interpreter, was a very powerful system and ran fast enough on the Apple II to be interactive but didn't win any speed contests. FORTH was the poster child for efficiency and obtuse syntax. Commonly referred to as a write only language, it was difficult to come up to speed as a developer, especially when using other's code. My own project in creating a Java VM for the Apple II uncovered the folly of shoehorning a large system into something never intended to run 32 bit applications.

##Low Level Implementation

Both the Pascal and Java VMs used a bytecode to hide the underlying CPU architecture and offer platform agnostic application execution. The application and tool chains were easily moved from platform to platform by simply writing a bytecode interpreter and small runtime to translate the higher level constructs to the underlying hardware. The performance of the system was dependent on the actual hardware and efficiency of the interpreter. Just-in-time compilation wasn't really an option on small, 8 bit systems. FORTH, on the other hand, was usually implemented as a threaded interpreter. A threaded interpreter will use the address of functions to call as the code stream instead of a bytecode, eliminating one level of indirection with a slight increase in code size.  The threaded approach can be made faster at the expense of another slight increase in size by inserting an actual Jump SubRoutine opcode before each address, thus removing the interpreter's inner loop altogether. 

All three systems were implemented using stack architecture.  Pascal and Java were meant to be compiled high level languages, using a stack machine as a simple compilation target.  FORTH was meant to be written directly as a stack oriented language, similar to RPN on HP calculators. The 6502 is a challenging target due to it's unusual architecture so writing a bytecode interpreter for Pascal and Java results in some inefficiencies and limitations.  FORTH's inner interpreter loop on the 6502 tends to be less efficient than most other CPUs.  Another difference is how each system creates and manipulates it's stack. Pascal and Java use the 6502 hardware stack for all stack operations. Unfortunately the 6502 stack is hard-limited to 256 bytes. However, in normal usage this isn't too much of a problem as the compilers don't put undue pressure on the stack size by keeping most values in global or local variables.  FORTH creates a small stack using a portion of the 6502's zero page, a 256 byte area of low memory that can be accessed with only a byte address and indexed using either of the X or Y registers. With zero page, the X register can be used as an indexed, indirect address and the Y register can be used as an indirect, indexed address.

##A New Approach

PLASMA takes an approach that uses the best of all the above implementations to create a unique, powerful and efficient platform for developing new applications on the Apple II. One goal was to create a very small VM runtime, bytecode interpreter, and module loader. The decision was made early on to implement a stack based architecture duplicating the approach taken by FORTH. Space in the zero page would be assigned to a 16 bit, 16 element evaluation stack, indexed by the X register.

A simple compiler was written so that higher level constructs could be used and global/local variables would hold values instead of using clever stack manipulation. Function/procedure frames would allow for local variables, but with a limitation - the frame could be no larger than 256 bytes. By enforcing this limitation, the function frame could easily be accessed through a frame pointer value in zero page, indexed by the Y register. The call stack uses the 6502's hardware stack resulting in the same 256 byte limitation imposed by the hardware. However, this limitation could be lifted by extending the call sequence to save and restore the return address in the function frame. This was not done initially for performance reasons and simplicity of implementation. Even with these limitations, recursive functions can be effectively implemented.

One of the goals of PLASMA was to allow for intermixing of functions implemented as bytecode, or native code. Taking a page from the FORTH play book, a function call is implemented as a native subroutine call to an address. If the function is in bytecode, the first thing it does is call back into the interpreter to execute the following bytecode (or a pointer to the bytecode). Function call parameters are pushed onto the evaluation stack in order they are written. The first operation inside of the function call is to pull the parameters off the evaluation stack and put them in local frame storage. Function callers and callees must agree on the number of parameters to avoid stack underflow/overflow. All functions return a value on the evaluation stack regardless of it being used or not.

The bytecode interpreter is capable of executing code in main memory or banked memory, increasing the available code space and relieving pressure on the limited 48K of data memory. In the Apple IIe with 64K expansion card, the IIc, and the IIgs, there is an auxilliary memory that swaps in and out for the main memory in chunks.  The interpreter resides in the Language Card memory area that can easily swap in and out the $0200 to $BFFF memory bank.  The module loader will move the bytecode into the auxilliary memory and fix up the entrypoints to reflect the bytecode location.

Lastly, PLASMA is not a typed language. Just like assembly, any value can represent a character, integer, or address. It's the programmer's job to know the type. Only bytes and words are known to PLASMA. Bytes are unsigned 8 bit quantities, words are signed 16 bit quantities. All stack operations involve 16 bits of precision.

The PLASMA low level operations are defined as:

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


##PLASMA Compiler/Assembler

Although the low-level operations could easily by coded by hand, they were chosen to be an easy target for a simple compiler. Think along the lines of an advanced assembler or stripped down C compiler ( C--).  Taking concepts from BASIC, Pascal, C and assembler, the PLASMA compiler is simple yet expressive. The syntax is line oriented; there is no statement delimiter except newline.

Comments are allowed throughout the source, starting with the ‘;’ character.  The rest of the line is ignored.

```
    ; Data and text buffer constants
```

Hexadecimal constants are preceded with a ‘$’ to identify them as such. 

```
    $C030 ; Speaker address
```

###Constants, Variables and Functions

The source code of a PLASMA module first defines imports, constants, variables and data.  Constants must be initialized with a value.  Variables can have sizes associated with them to declare storage space.  Data can be declared with or without a variable name associated with it.  Arrays, tables, strings and any predeclared data can be created and accessed in multiple ways.

```
    ;
    ; Import standard library functions.
    ;
    import stdlib
        predef putc, puts, getc, gets, cls, memcpy, memset, memclr
    end
    ;
    ; Constants used for hardware and flags
    ;
    const speaker     = $C030
    const changed     = 1
    const insmode     = 2
    ;
    ; Array declaration of screen row addresses
    ;
    word  txtscrn[]   = $0400,$0480,$0500,$0580,$0600,$0680,$0700,$0780
    word              = $0428,$04A8,$0528,$05A8,$0628,$06A8,$0728,$07A8
    word              = $0450,$04D0,$0550,$05D0,$0650,$06D0,$0750,$07D0
    ;
    ; Misc global variables
    ;
    byte  flags       = 0
    word  numlines    = 0
    byte  cursx, cursy
    word  cursrow, scrntop, cursptr
```

Variables can have optional brackets; empty brackets don’t reserve any space for the variable but are useful as a label for data that is defined following the variable.  Brackets with a constant inside defines a minimum size reserved for the variable.  Any data following the variable will take at least the amount of reserved space, but potentially more.

Strings are defined like Pascal strings, a length byte followed by the string characters so they can be a maximum of 255 characters long.  Strings can only appear in the variable definitions of a module.  String constants can’t be used in expressions or statements.

```
    ;
    ; An initialized string of 64 characters
    ;
    byte txtfile[64] = "UNTITLED"
```

Functions are defined after all constants, variables and data.  Functions can be forward declared with a *predef* type in the constant and variable declarations.  Functions have optional parameters and always return a value.   Functions can have their own variable declarations.  However, unlike the global declarations, no data can be predeclared, only storage space.  There is also a limit of 254 bytes of local storage.  Each parameter takes two bytes of local storage, plus two bytes for the previous frame pointer.  If a function has no parameters or local variables, no local frame will be created, improving performance.  A function can specify a value to return.  If no return value is specified, a default of 0 will be returned.

After functions are defined, the main code for the module follows. The main code will be executed as soon as the module is loaded.  For library modules, this is a good place to do any runtime initialization, before any of the exported functions are called. The last statement in the module must be done, or else a compile error is issued.

There are four basic types of data that can be manipulated: constants, variables, addresses, and functions.  Memory can only be read or written as either a byte or a word.  Bytes are unsigned 8 bit quantities, words are signed 16 bit quantities.  Everything on the evaluation stack is treated as a word.  Other than that, any value can be treated as a pointer, address, function, character, integer, etc.  There are convenience operations in PLASMA to easily manipulate addresses and expressions as pointers, arrays, structures, functions, or combinations thereof.  If a variable is declared as a byte, it can be accessed as a simple, single dimension byte array by using brackets to indicate the offset.  Any expression can calculate the indexed offset.  A word variable can be accessed as a word array in the same fashion.  In order to access expressions or constants as arrays, a type identifier has to be inserted before the brackets.  a ‘.’ character denotes a byte type, a ‘:’ character denotes a word type.  Along with brackets to calculate an indexed offset, a constant can be used after the ‘.’ or ‘:’ and will be added to the base address.  The constant can be a defined const to allow for structure style syntax.  If the offset is a known constant, using the constant offset is a much more efficient way to address the elements over an array index.  Multidimensional arrays are treated as arrays of array pointers.  Multiple brackets can follow the ‘.’ or ‘:’ type identifier, but all but the last index will be treated as a pointer to an array.

```
    word hgrscan[] = $2000,$2400,$2800,$2C00,$3000,$3400,$3800,$3C00
    word           = $2080,$2480,$2880,$2C80,$3080,$3480,$3880,$3C80

    hgrscan:[yscan][xscan] = fillval
```

Values can be treated as pointers by preceding them with a ‘^’ for byte pointers, ‘*’ for word pointers.

```
    strlen = ^srcstr
```

Addresses of variables and functions can be taken with a preceding ‘@’, address-of operator. Parenthesis can surround an expression to be used as a pointer, but not address-of.

Functions can have optional parameters when called and local variables.  Defined functions without parameters can be called simply, without any paranthesis.

```
    def drawscrn(topline, leftpos)
        byte i
        for i = 0 to 23
            drawline(textbuff[i + topline], leftpos)
        next
    end
    def redraw
        cursoff
        drawscrn(scrntop, scrnleft)
        curson
    end

    redraw
```

Functions with parameters or expressions to be used as a function address to call must use parenthesis, even if empty.

```
    predef keyin2plus
    word keyin
    byte key

    keyin = @keyin2plus ; address-of keyin2plus function
    key   = keyin()
```

Expressions and Statements

Expressions are algebraic.  Data is free-form, but all operations on the evaluation stack use 16 bits of precision with the exception of byte load and stores.  A stand-alone expression will be evaluated and read from or called.  This allows for easy access to the Apple’s soft switches and other memory mapped hardware. The value of the expression is dropped.

```
    const speaker=$C030

    ^speaker ; click speaker
    close(refnum)
```

More complex expressions can be built up using algebraic unary and binary operations.

| OP   |     Unary Operation |
|:----:|---------------------|
| ^    | byte pointer
| *    | word pointer
| @    | address of
| -    | negate
| ~    | bitwise compliment
| NOT  | logical NOT


| OP   |     Binary Operation |
|:----:|----------------------|
| *    | multiply
| /    | divide
| %    | modulo
| +    | add
| -    | subtract
| <<   | shift left
| >>   | shift right
| &    | bitwise AND
| ^    | bitwise XOR
| &#124; | bitwise OR
| ==   | equals
| <>   | not equal
| >=   | greater than or equal
| >    | greater than
| <=   | less than or equal
| <    | less than
| OR   |  logical OR
| AND  |  logical AND

Statements are built up from expressions and control flow keywords.  Simplicity of syntax took precedence over flexibility and complexity.  The simplest statement is the basic assignment using ‘=’.

```
    byte numchars
    numchars = 0
```

Expressions can be built up with constants, variables, function calls, addresses, and pointers/arrays.  Comparison operators evaluate to 0 or -1 instead of the more traditional 0 or 1.  The use of -1 allows binary operations to be applied to other non-zero values and still retain a non-zero result.  Any conditional tests check only for zero and non-zero values.

Control structures affect the flow of control through the program.  There are conditional and looping constructs.  The most widely used is probably the if/elsif/else/fin construct.

```
    if ^pushbttn3 < 128
        if key == $C0
            key = $D0 ; P
        elsif key == $DD
            key = $CD ; M
        elsif key == $DE
            key = $CE ; N
        fin
    else
       key = key | $E0
    fin
```

The when/is/otherwise/wend statement is similar to the if/elsif/else/fin construct except that it is more efficient.  It selects one path based on the evaluated expressions, then merges the code path back together at the end.  However only the 'when' value is compared against a list of expressions.  The expressions do not need to be constants, they can be any valid expression.  The list of expressions is evaluated in order, so for efficiency sake, place the most common cases earlier in the list.

```
    when keypressed
        is keyarrowup
            cursup
        is keyarrowdown
            cursdown
        is keyarrowleft
            cursleft
        is keyarrowright
            cursright
        is keyctrlx
            cutline
        is keyctrlv
            pasteline
        is keyescape
            cursoff
            cmdmode
            redraw
        otherwise
            bell
    wend
```

The most common looping statement is the for/next construct.

```
    for xscan = 0 to 19
        (scanptr):[xscan] = val
    next
```

The for/next statement will efficiently increment or decrement a variable form the starting value to the ending value.  The increment/decrement amount can be set with the step option after the ending value; the default is one.  If the ending value is less than the starting value, use downto instead of to to progress in the negative direction.  Only use positive step values.  The to or downto will add or subtract the step value appropriately.

```
    for i = heapmapsz - 1 downto 0
        if sheapmap.[i] <> $FF
            mapmask = szmask
        fin
    next
```

while/loop statements will continue looping as long as the while expression is non-zero.

```
    while !(mask & 1)
        addr = addr + 16
        mask = mask >> 1
    loop
```

Lastly, the repeat/until statement will continue looping as long as the until expression is zero.

```
    repeat
        txtbuf   = read(refnum, @txtbuf + 1, maxlnlen)
        numlines = numlines + 1
    until txtbuf == 0 or numlines == maxlines
```

###Runtime

PLASMA includes a very minimal runtime that nevertheless provides a great deal of functionality to the system.  Two system calls are provided to access native 6502 routines (usually in ROM) and ProDOS.

romcall(aReg, xReg, yReg, statusReg, addr) returns a pointer to a four byte structure containing the A,X,Y and STATUS register results.

```
    const xreg = 1
    const getlin = $FD6A

    numchars = (romcall(0, 0, 0, 0, getlin)).xreg ; return char count in X reg
```

syscall(cmd, params) calls ProDOS, returning the status value.

```
    def read(refnum, buff, len)
        byte params[8]

        params.0 = 4
        params.1 = refnum
        params:2 = buff
        params:4 = len
        perr     = syscall($CA, @params)
        return params:6
    end
```

putc(char), puts(string), home, gotoxy(x,y), getc() and gets() are other handy utility routines for interacting with the console.

```
    putc('.')
    byte okstr[] = "OK"
    puts(@okstr)
```

memset(addr, len, val) will fill memory with a 16 bit value.  memcpy(dstaddr, srcaddr, len) will copy memory from one address to another, taking care to copy in the proper direction.

```
    byte nullstr[] = ""
    memset(strlinbuf, maxfill * 2, @nullstr) ; fill line buff with pointer to null string
    memcpy(scrnptr, strptr + ofst + 1, numchars)
```

##Implementation Details
###The Original PLASMA
The original design concept was to create an efficient, flexible, and expressive environment for building applications directly on the Apple II.  Choosing a stack based architecture was easy after much experience with other stack based implementations. It also makes the compiler simple to implement.  The first take on the stack architecture was to make it a very strict stack architecture in that everything had to be on the stack.  The only opcode with operands was the CONSTANT opcode. This allowed for a very small bytecode interpreter and a very easy compile target.  However, only when adding an opcode with operands that would greatly improved performance, native code generation or code size was it done. The opcode table grew slowly over time but still retains a small runtime interpreter with good native code density.

The VM was constructed such that code generation could ouput native 6502 code, threaded code into the opcode functions, or interpreted bytecodes.  This gave a level of control over speed vs memory.

###The Lawless Legends PLASMA
This version of PLASMA has dispensed with the native/threaded/bytecode code generation from the original version to focus on code density and the ability to interpret bytecode from AUX memory, should it be available. By focussing on the bytecode interpreter, certain optimizations were implemented that weren't posssible when allowing for threaded/native code; the interpreted bytecode is now about the same performance as the directly threaded code.

Dynamically loadable modules, a backward compatible extension to the .REL format introduced by EDASM, is the new, main feature for this version of PLASMA. A game like Lawless Legends will push the capabilities of the Apple II well beyond anything before it. A powerful OS + language + VM environment is required to achieve the goals set out.

## References
PLASMA User Manual: https://github.com/badvision/lawless-legends/blob/master/Docs/Tutorials/PLASMA/User%20Manual.md

B Programming Language User Manual  http://cm.bell-labs.com/cm/cs/who/dmr/kbman.html

FORTH   http://en.wikipedia.org/wiki/Forth_(programming_language)

UCSD Pascal http://wiki.freepascal.org/UCSD_Pascal

p-code  https://www.princeton.edu/~achaney/tmve/wiki100k/docs/P-code_machine.html

VM02: Apple II Java VM  http://sourceforge.net/projects/vm02/

Threaded code   http://en.wikipedia.org/wiki/Threaded_code
