; Include some useful misc and act like a standard library
; Upgraded from a demo: 13:20, 2017.11.2 by liftA42.

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

; cannot use `list-foldr` directly, because eval order must be from left to
; right
(define list-map (lambda (f lst)
  ((lambda (reversed-mapped)
    (list-reverse reversed-mapped)
  ) (list-foldl (lambda (acc item) (cons (f item) acc)) nil lst))
))

(define foldl (lambda (f init)
  (pipe list (lambda (lst) (list-foldl f init lst)))
))

(define list-index (lambda (i lst)
  (cond ((nil? lst)
    nil)
  ((= i 0)
    (car lst))
  (else
    (list-index (- i 1) (cdr lst))
  ))
))

(define quoted-example (quote list
  (this is an (quote list example list and) (the (number is) 42.))
))

(define literal-example (quote list 'Hello, liftA42!'))

(define list-print-string (lambda (str)
  (list-map display-char str)
  (newline)
))

(define list-append (lambda (lst x)
  (cond ((nil? lst)
    (cons x nil))
  (else
    (cons (car lst) (list-append (cdr lst) x))))
))

(list-print-string literal-example)
