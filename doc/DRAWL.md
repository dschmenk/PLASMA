# LISP 1.5 implemented in PLASMA

LISP interpreted on a bytecode VM running on a 1 MHz 6502 is going to be sssllllooooowwwww. So I called this implementation DRAWL in keeping with the speech theme. DRAWL represents an exploration REPL language for the PLASMA environment. It isn't meant to be a full-blown programming language, more of an interactive sandbox for playing with S-expressions.

## Missing features of LISP 1.5 in DRAWL

- General recursion. The 6502 architecture limits recursion (but see tail recursion below), so don't expect too much here

However, the code is partitioned to allow for easy extension so some of these missing features could be implemented.

## Features of DRAWL

- 32 bit integers and 80 bit floating point with transcendental math operators by way of the SANE library
- Tail recursion handles deep recursion. Check out [loop.lisp](https://github.com/dschmenk/PLASMA/blob/master/src/lisp/loop.lisp)
- Fully garbage collected behind the scenes
- Optionally read LISP source file at startup
- The PROG feature now present!
- Arrays of up to four dimensions
- FUNCTION operation with bound variables
- Additional testing/looping construct: IF, FOR, WHILE, UNTIL
- Bit-wise logic operations on 32 bit integers
- Hexadecimal input/output

The DRAWL implementation comes with the follwoing built-in:

### Constants
- T = True
- F = False
- NIL = NULL
- SPACE = SPACE character on output
- CR = Carriage Return character on output
- CSET()
- CSETQ()
- DEFINE()

### Function types

- LAMBDA(...)
- FUNARG() = List constructed by FUNCTION()
- FUNCTION()


### Predicates

- ATOM()
- EQ()
- NOT()
- AND(...)
- OR(...)
- NULL()

### Misc

- SET
- QUOTE()
- ARRAY()
- TRACE() = Turn tracing on/off
- GC() = Run garbage collector and return free memory amount
- QUIT() = Exit REPL

### List manipulation

- CAR()
- CDR()
- CONS()
- LIST(...)

### Conditionals

- COND(...)
- IF()

### Output

- PRHEX() = Turn hexadecimal output on/off
- PRI(...) = Print without newline
- PRINT(...) = Print with newline
- FMTFPI() = Floating point format integer digits
- FMTFPF() = Floating point format fractional digits
- PRINTER() = Turn printer echo on/off on slot#

### Looping

- FOR()
- WHILE()
- UNTIL()

### Associations

- LABEL()
- SET()
- SETQ()

### Program feature

- PROG() = Algol like programming in LISP
- GO() = Goto label inside PROG
- RETURN() = Return from PROG with value

### Numbers

- +(...)
- -()
- \*()
- /()
- REM()
- NEG()
- ABS()
- >()
- <()
- MIN(...)
- MAX(...)

### Integers

- BITNOT() = Bit-wise NOT
- BITAND() = Bit-wise AND
- BITOR() = Bit-wise OR
- BITXOR= Bit-wise XOR
- SHIFT() = Bit-wise SHIFT (positive = left, negative = right)
- ROTATE() = Bit-wise ROTATE (positive = left, negative = right)

### Floating Point (from the SANE library)

- PI()
- MATH_E()
- LOGB()
- SCALEB_I()
- TRUNCATE()
- ROUND()
- SQRT()
- COS()
- SIN()
- TAN()
- ATAN()
- LOG2()
- LOG2_1()
- LN()
- LN_1()
- POW2()
- POW2_1()
- POWE()
- POWE_1()
- POWE2_1()
- POW_I()
- POWY()
- COMP()
- ANNUITY()

LISP is one of the earliest computer languages. As such, it holds a special place in the anals of computer science. I've always wanted to learn why LISP is held in such high regard by so many, so I went about learning LISP by actually implementing a LISP interpreter in PLASMA. PLASMA is well suited to implement other languages due to its rich syntax, performance and libraries.

## Links

Here are some links to get you started.

LISP 1.5 Manual: https://archive.org/details/bitsavers_mitrlelisprammersManual2ed1985_9279667

LISP 1.5 Primer: https://www.softwarepreservation.org/projects/LISP/book/Weismann_LISP1.5_Primer_1967.pdf

Apple Numerics Manual (SANE): https://vintageapple.org/inside_o/pdf/Apple_Numerics_Manual_Second_Edition_1988.pdf

Video showing DRAWL in action: https://youtu.be/wBMivg6xfSg

Preconfigured PLASMA ProDOS boot floppy for DRAWL: https://github.com/dschmenk/PLASMA/blob/master/images/apple/DRAWL.po
