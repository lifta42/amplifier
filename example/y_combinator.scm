(define Y (lambda (f)
  (lambda (x) (f f x))))

(define part-fac (lambda (self n)
  (cond
    ((= n 1) 1)
    (else (* n (self self (- n 1)))))))

(define print-line (lambda (p) (display p) (newline)))

(print-line ((Y part-fac) 5))

(define part-fib (lambda (self n)
  (cond
    ((= n 1) 1)
    ((= n 2) 1)
    (else (+ (self self (- n 1)) (self self (- n 2)))))))

(print-line ((Y part-fib) 5))
