# LISP 1.5 implemented in PLASMA

LISP interpreted on a bytecode VM running on a 1 MHz 6502 is going to be sssllllooooowwwww. So I called this implementation DRAWL in keeping with the speech theme. DRAWL represents an exploration REPL language for the PLASMA environment. It isn't meant to be a full-blown programming language, more of an interactive sandbox for playing with S-expressions.

## Missing features of LISP 1.5 in DRAWL

- FUNCTION operation. Use QUOTE for functions that don't use higher up bound variables
- General recursion. The 6502 architecture limits recursion (but see tail recursion below), so don't expect too much here

However, the code is partitioned to allow for easy extension so some of these missing features could be implemented.

## Features of DRAWL

- 32 bit integers and 80 bit floating point with transcendental math operators by way of the SANE library
- Tail recursion handles deep recursion. Check out [loop.lisp](https://github.com/dschmenk/PLASMA/blob/master/src/lisp/loop.lisp)
- Fully garbage collected behind the scenes
- Optionally read LISP source file at startup
- The PROG feature now present!
- Arrays of up to four dimensions

LISP is one of the earliest computer languages. As such, it holds a special place in the anals of computer science. I've always wanted to learn why LISP is held in such high regard by so many, so I went about learning LISP by actually implementing a LISP interpreter in PLASMA. PLASMA is well suited to implement other languages due to its rich syntax, performance and libraries.

## Links

Here are some links to get you started.

LISP 1.5 Manual: https://archive.org/details/bitsavers_mitrlelisprammersManual2ed1985_9279667

Video showing DRAWL in action: https://youtu.be/wBMivg6xfSg

Preconfigured PLASMA ProDOS boot floppy for DRAWL: https://github.com/dschmenk/PLASMA/blob/master/images/apple/DRAWL.po
