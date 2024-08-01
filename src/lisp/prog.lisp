(define
  (lengthc (lambda (l)
    ;
    ; Use cond() inside prog()
    ;
    (prog (u v)
      (setq v 0)
      (setq u l)
    a (cond ((null u),(return v)))
      (setq v (+ 1 v))
      (setq u (cdr u))
      (go a)
     )
  ))
  (lengthi (lambda (l)
    ;
    ; Use if/then/else inside prog()
    ;
    (prog (u v)
      (setq v 0)
      (setq u l)
    a (if (null u) (return v) (setq v (+ 1 v)))
      (setq u (cdr u))
      (go a)
     )
   ))
)
