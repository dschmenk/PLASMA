(label fact
         (lambda (n)
           (cond
             ((eq n 0) , 1)
             (t , (* n (fact (- n 1))))
           )
         )
)
