(define +& (lambda (x y k) (k (+ x y))))
(define -& (lambda (x y k) (k (- x y))))
(define *& (lambda (x y k) (k (* x y))))

(define if& (lambda (c f g)
  (cond
    (c (f))
    (else (g)))))

(define fac& (lambda (n k)
  (if& (= n 1)
    (lambda () (k 1))
    (lambda ()
      (-& n 1 (lambda (n-dec)
        (fac& n-dec (lambda (fac-dec)
          (*& fac-dec n k)))))))))

(define print-line& (lambda (s k) (display s) (newline) (k)))

(define feed (lambda (f& k)
  (lambda (x) (f& x k))))

(fac& 5 (feed print-line& exit))
