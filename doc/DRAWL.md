# LISP 1.5 implemented in PLASMA

LISP interpreted on a bytecode VM running on a 1 MHz 6502 is going to be sssllllooooowwwww. So I called this implementation DRAWL in keeping with the speech theme. DRAWL represents an exploration REPL language for the PLASMA environment. It isn't meant to be a full-blown programming language, more of an interactive sandbox for playing with S-expressions.

## Missing features of LISP 1.5 in DRAWL

- Minimal I/O facilities
- Property lists. Only DEFINE() and CSETQ()/CSET() property functions supported
- General recursion. The 6502 architecture limits recursion (but see tail recursion below), so don't expect too much here
- Many of the built-in functions from the LISP 1.5 manual. Most can be coded in LISP and loaded at startup

However, the code is partitioned to allow for easy extension so some of these missing features could be implemented.

## Features of DRAWL

- 32 bit integers and 80 bit floating point with transcendental math operators by way of the SANE library
- Tail recursion handles deep recursion. Check out [loop.lisp](https://github.com/dschmenk/PLASMA/blob/master/src/lisp/loop.lisp)
- Fully garbage collected behind the scenes
- Optionally read LISP source file at startup
- Arrays of up to four dimensions
- Bit-wise logic operations on 32 bit integers
- FUNCTION operation with bound variables
- The PROG Alogol-like programming construct
- Additional testing/looping constructs: IF and FOR
- Hexadecimal input/output
- LoRes Apple II graphics
- Ctrl-C break into running program
- MACROs for meta-programming. See [defun.lisp](https://github.com/dschmenk/PLASMA/blob/master/src/lisp/defun.lisp)
- End-of-line comment using ';'
- String handling functions

The DRAWL implementation comes with the following built-in functions:

### Constants

- T = True
- F = False
- NIL = NULL
- CSET() = Set constant value
- CSETQ() = Set constant value
- DEFINE() = Define function

### Function types

- LAMBDA(...)
- FUNARG() = List constructed by FUNCTION()
- FUNCTION()
- MACRO(...) = Operate on non-evaluated argument list

### Predicates

- ATOM()
- EQ()
- NOT()
- AND(...)
- OR(...)
- NULL()
- NUMBERP()
- STRINGP()

### Misc

- SET = Used in array access to set element value
- QUOTE()
- ARRAY() = Arrays up to four dimensions
- EVAL() = Evaluate S-expression
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
- IF() = IF THEN w/ optional ELSE

### Output

- PRHEX() = Turn hexadecimal output on/off
- PRI(...) = Print without newline
- PRINT(...) = Print with newline
- FMTFPI() = Floating point format integer digits
- FMTFPF() = Floating point format fractional digits
- PRINTER() = Turn printer echo on/off on slot#

### Looping

- FOR(...)

### Associations

- LABEL()
- SET()
- SETQ()

### Program feature

- PROG(...) = Algol like programming in LISP
- SET() = Update variable value
- SETQ() = Update variable value
- COND(...) = Fall-through COND()
- IF() = Fall-through IF THEN w/ optional ELSE
- GO() = Goto label inside PROG
- RETURN() = Return from PROG with value

### Numbers

- +(...)
- -()
- \*(...)
- /()
- REM()
- NEG()
- ABS()
- \>()
- <()
- MIN(...)
- MAX(...)
- NUMBERP()

### Integers

- BITNOT() = Bit-wise NOT
- BITAND() = Bit-wise AND
- BITOR() = Bit-wise OR
- BITXOR= Bit-wise XOR
- ARITHSHIFT() = Bit-wise arithmetic SHIFT (positive = left, negative = right)
- LOGICSHIFT() = Bit-wise logicalal SHIFT (positive = left, negative = right)
- ROTATE() = Bit-wise ROTATE (positive = left, negative = right)

### Floating Point (from the SANE library)

- PI() = Constant value of pi
- MATH_E() = Constant value of e
- NUMBER() = Convert atom to number (symbol and array return NIL)
- INTEGER() = Convert number to integer
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
- POW_I()
- POW()
- COMP()
- ANNUITY()

### Strings

- STRING() = Convert atom to string
- SUBS() = SUB String offset length
- CATS(...) = conCATenate Strings
- LENS() = LENgth String
- CHARS() = CHARacter String from integer value
- ASCII() = ASCII value of first character in string

### I/O functions

- HOME()
- GOTOXY()
- KEYPRESSED()
- READKEY()
- READ()
- READFILE()
- READSTRING()

### Lo-Res Graphics

- GR() = Turn lo-res graphics mode on/off
- COLOR() = Set plotting color
- PLOT() = Plot pixel at X,Y coordinate

LISP is one of the earliest computer languages. As such, it holds a special place in the anals of computer science. I've always wanted to learn why LISP is held in such high regard by so many, so I went about learning LISP by actually implementing a LISP interpreter in PLASMA. PLASMA is well suited to implement other languages due to its rich syntax, performance and libraries.

## Links

Here are some links to get you started.

LISP 1.5 Manual: https://archive.org/details/bitsavers_mitrlelisprammersManual2ed1985_9279667

LISP 1.5 Primer: https://www.softwarepreservation.org/projects/LISP/book/Weismann_LISP1.5_Primer_1967.pdf

Personal LISP on Apple II Manual (PDF): https://archive.org/details/gLISP/gnosisLISPManual

Personal LISP on Apple II Manual (web archive): https://web.archive.org/web/20190603120105/http://jeffshrager.org/llisp/

Apple Numerics Manual (SANE): https://vintageapple.org/inside_o/pdf/Apple_Numerics_Manual_Second_Edition_1988.pdf

Part 1 of DRAWL in action (S-expressions): https://youtu.be/wBMivg6xfSg

Part 2 of DRAWL in action (The rest of LISP 1.5): https://youtu.be/MdKZIrfPN7s

Preconfigured PLASMA ProDOS boot floppy for DRAWL: https://github.com/dschmenk/PLASMA/blob/master/images/apple/DRAWL.po

My blog post about LISP 1.5 and DRAWL: http://schmenk.is-a-geek.com/wordpress/?p=365
