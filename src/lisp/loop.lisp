(DEFINE
  (LOOP (LAMBDA (I M FN)
    (COND ((AND (< I M) (FN I)),(LOOP (+ 1 I) M FN))
          (T,(EQ I M)))
  ))
  (LPRINT (LAMBDA (N)
    (ATOM (PRINT N))
  ))
)

(LOOP 1 100 LPRINT)
