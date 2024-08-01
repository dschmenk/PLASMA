;
; Recursive factorial
;
(define
  (fact (lambda (n)
    (cond ((eq n 0) , 1)
          (t , (* n (fact (- n 1))))
    )
  ))
)

(fact 1)
(fact 2)
(fact 3)
(fact 4)
(fact 5)
