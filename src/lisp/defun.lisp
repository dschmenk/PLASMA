;
; USE MACRO TO SIMPLIFY FUNCTION DEFINITION
;
(DEFINE (CADR  (LAMBDA (L) (CAR (CDR L))))
        (CDDR  (LAMBDA (L) (CDR (CDR L))))
        (CADDR (LAMBDA (L) (CAR (CDR (CDR L)))))
        (CDDDR (LAMBDA (L) (CDR (CDR (CDR L)))))
        (DEFUN (MACRO (L)
          (EVAL (CONS 'DEFINE
               (LIST (CONS (CAR L) (LIST (CONS 'LAMBDA (CDR L)))))))
        ))
        (DEFPRO (MACRO (L)
          (EVAL (CONS 'DEFINE
               (LIST (CONS (CAR L)
                 (LIST (CONS 'LAMBDA (LIST (CADR L)
                   (CONS 'PROG (CDDR L))
                 )))
               ))
          ))
        ))
)
