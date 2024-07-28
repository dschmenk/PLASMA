(DEFINE
  (TAILLOOP (LAMBDA (I M)
    (COND ((AND (< I M) (PRIN I)),(TAILLOOP (+ 1 I) M))
          (T,(- I 1)))
  ))
  (PROGLOOP (LAMBDA (I M)
    (PROG ()
    A (PRIN I)
      (SETQ I (+ I 1))
      (IF (< I M) (GO A))
      (RETURN (- I 1))
  )))
  (FORLOOP (LAMBDA (I M)
    (FOR I I 1 (< I M) (PRIN I))
  ))
)

'TAIL
(TAILLOOP 1 100)
'PROG
(PROGLOOP 1 100)
'FOR
(FORLOOP 1 100)
