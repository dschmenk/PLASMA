(DEFINE
  (TAILLOOP (LAMBDA (I M)
    (COND ((AND (< I M) (PRIN I)),(TAILLOOP (+ 1 I) M))
          (T,I))
  ))
  (PROGLOOP (LAMBDA (I M)
    (PROG ()
    A (PRIN I)
      (SETQ I (+ I 1))
      (IF (< I M) (GO A))
      (RETURN I)
  )))
  (FORLOOP (LAMBDA (I M)
    (FOR I 1 1 (< I M) (PRIN I))
  ))
)

'TAIL
(TAILLOOP 1 100)
'PROG
(PROGLOOP 1 100)
'FOR
(FORLOOP 1 100)
