(define cons (lambda (h t) (lambda (f) (f h t))))
(define car (lambda (c) (c (lambda (h t) h))))
(define cdr (lambda (c) (c (lambda (h t) t))))

(define print-line (lambda (s) (display s) (newline)))

(define list (foldr (lambda (acc item) (cons item acc)) nil))

(define list-example (list 1 2 3 4))

(define list-map (lambda (f)
  (define iter (lambda (lst)
    (cond
      ((nil? lst) nil)
      (else
        ((lambda (h)
          (cons h (iter (cdr lst))))
        (f (car lst)))))))
  iter))

(define list-display (list-map print-line))

(list-display list-example)
