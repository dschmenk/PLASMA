;
; Rod's Colors
;
(define (rod (lambda () (prog
       (i j k w fmi fmk clr) ; local vars

       (gr t)
       (gotoxy 11 1)
       (print "Press any key to exit.")
  loop (for w 3 1 (< w 51) ; for w = 3 to 50
       (for i 1 1 (< i 20) ; for i = 1 to 19
       (for j 0 1 (< j 20) ; for j = 0 to 19
          (= k (+ i j))    ; k = i + j
          (= clr (+ (/ (* j 3) (+ i 3)) (/ (* i w) 12)))
                           ; clr = (j * 3) / (i + 3) + i * w / 12
          (= fmi (- 40 i)) ; fmi = 40 - i
          (= fmk (- 40 k)) ; fmk = 40 - k
          (color clr)      ; conio:grcolor(clr)
          (plot i k)       ; conio:grplot(i, k)
          (plot k i)       ; conio:grplot(k, i)
          (plot fmi fmk)   ; conio:grplot(fmi, fmk)
          (plot fmk fmi)   ; conio:grplot(fmk, fmi)
          (plot k fmi)     ; conio:grplot(k, fmi)
          (plot fmi k)     ; conio:grplot(fmi, k)
          (plot i fmk)     ; conio:grplot(i, fmk)
          (plot fmk i)     ; conio:grplot(fmk, i)
          (if (keypressed?) (return (and (readkey) (gr f))))
                           ; if conio:keypressed()
                           ;  conio:getkey()
                           ;  return
                           ; fin
        )                  ; next
      )                    ; next
    )                      ; next
    (go loop)              ; loop
))))                       ; end

(rod)
"That's all, folks!"
