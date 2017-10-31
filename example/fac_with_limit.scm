;; First multiply, then add when result is greater than limitation.
;; 1 * 2 * 3 * 4 * 5 + 6 + 7 if limit == 100

(define fac-with-limit (lambda (lmt)
  (define fwl (lambda (n)
    (cond
      ((= n 1) 1)
      (else
        ((lambda (m)
          ((cond
            ((> m lmt) +)
            (else *))
          m n))
        (fwl (- n 1)))))))
  fwl))

(display ((fac-with-limit 100) 7))
