;
; Sample from LISP 1.5 Manual
;
(define
  (ydot
    (lambda (x y)
      (maplist x (function (lambda (j) (cons (car j) y))))
    )
  )
  (maplist
    (lambda (l fn)
      (cond
        ((null l) , nil)
        (t , (cons (fn l) (maplist (cdr l) fn)))
      )
    )
  )
)

'(ydot '(a (b.c) d e) 'z)
(ydot '(a (b.c) d e) 'z)
