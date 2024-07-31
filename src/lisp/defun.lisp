(DEFINE (DEFUN (MACRO (L)
   (EVAL (CONS 'DEFINE
               (LIST (CONS (CAR L) (LIST (CONS 'LAMBDA (CDR L)))))))
)))
