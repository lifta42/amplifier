(define the-number 42)

(define add-the-number (lambda (x) (+ x the-number)))

(define id (lambda (x) x))

(define the-name-in-test (with (quote id test2) (lambda ()
  the-name
)))
