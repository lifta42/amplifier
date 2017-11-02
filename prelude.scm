(define cons (lambda (h t) (lambda (f) (f h t))))
(define car (lambda (c) (c (lambda (h t) h))))
(define cdr (lambda (c) (c (lambda (h t) t))))

(define print-line (lambda (s) (display s) (newline)))

(define list (foldr (lambda (acc item) (cons item acc)) nil))

(define list-example (list 1 2 3 4))

(define list-foldr (lambda (f init lst)
  (define iter (lambda (lst)
    (cond ((nil? lst)
      init)
    (else
      ((lambda (tail-acc)
        (f tail-acc (car lst))
      ) (iter (cdr lst)))
    ))
  ))
  (iter lst)
))

; https://stackoverflow.com/questions/6172004/writing-foldl-using-foldr
(define id (lambda (x) x))

(define list-foldl (lambda (f init lst)
  ((lambda (step)
    ((list-foldr step id lst) init)
  ) (lambda (g x) (lambda (a) (g (f a x)))))
))

(define list-reverse (lambda (lst)
  (list-foldl cons nil lst)
))

; cannot use `list-foldr` directly, because eval order must be left to right
(define list-map (lambda (f lst)
  ((lambda (reversed-mapped)
    (list-reverse reversed-mapped)
  ) (list-foldl (lambda (acc item) (cons (f item) acc)) nil lst))
))

(define list-display (lambda (lst) (list-map print-line lst)))

(define foldl (lambda (f init)
  (pipe list (lambda (lst) (list-foldl f init lst)))
))
