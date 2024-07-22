((label 
  gcd (lambda (x y)
    (cond ((> x y) , (gcd y x))
          ((eq (rem y x) 0) , x)
          (t , (gcd (rem y x) x))
    )
  )) 22 100
)
