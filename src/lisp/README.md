# DRAWL (LISP 1.5) S-expression processor

These are the files to implement a LISP 1.5 s-expression parser and evaluator. The PLASMA code is broken up into two modules: the core s-expression processor and the REPL environment.

s-expr.pla is the guts of the system. Some features missing from the LISP 1.5 manual include floating point numbers, PROG functions and arrays.

drawl.pla because this is a sslloowwww implementation of LISP, so named DRAWL in keeping with the speech theme. The file reading and REPL functions are contained here. DRAWL is meant to be extensible so adding PROG and floating point is easily possible.

The sample LISP code comes from the LISP 1.5 manual. These are the only LISP programs that have been run on DRAWL, so other LISP programs may uncover bugs or limitations of DRAWL.

More information and links can be found here: https://github.com/dschmenk/PLASMA/blob/master/doc/DRAWL.md
