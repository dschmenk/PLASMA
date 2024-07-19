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
