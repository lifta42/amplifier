(define cons (lambda (h t) (lambda (f) (f h t))))
(define car  (lambda (c) (c (lambda (h t) h))))
(define cdr  (lambda (c) (c (lambda (h t) t))))

(define simple-list (cons 1 (cons 2 (cons 4 (cons 8 nil)))))
(display (car (cdr simple-list)))
(newline)

(define list-map (lambda (f)
  (define iter (lambda (lst)
    (cond
      ((nil? lst) nil)
      (else
        ((lambda (h)
          (cons h (iter (cdr lst))))
        (f (car lst)))))))
  iter))

(define list-display
  (list-map (lambda (ele) (display ele) (newline) ele)))

(list-display simple-list)
