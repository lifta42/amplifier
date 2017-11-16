; Include some useful misc and act like a standard library
; Upgraded from a demo: 13:20, 2017.11.2 by liftA42.

(define cons (lambda (h t) (lambda (f) (f h t)) ))
(define car (lambda (c) (c (lambda (h t) h)) ))
(define cdr (lambda (c) (c (lambda (h t) t)) ))

(define id (lambda (x) x))

(define sum (foldr + 0))

; (quote cons '\n') ==> (quote cons (<nl> nil)) ==> (cons <nl> nil)
(define flip (lambda (func)
  (lambda (a b) (func b a))
))

(define list (foldr (flip cons) nil) )

(define list-example (list 1 2 3 4) )

(define list-foldr (lambda (f init lst)
  (define iter (lambda (lst)
    (if (nil? lst) (lambda ()
      init
    ) (lambda ()  ; else
      ((lambda (tail-acc)
        (f tail-acc (car lst))
      ) (iter (cdr lst)))
    ))
  ))
  (iter lst)
))

; https://stackoverflow.com/questions/6172004/writing-foldl-using-foldr
(define list-foldl (lambda (f init lst)
  ((lambda (step)
    ((list-foldr step id lst) init)
  ) (lambda (g x) (lambda (a) (g (f a x)))))
))

(define list-reverse (lambda (lst)
  (list-foldl (flip cons) nil lst)
))

; cannot use `list-foldr` directly, because eval order must be from left to
; right
(define list-map (lambda (f lst)
  ((lambda (reversed-mapped)
    (list-reverse reversed-mapped)
  ) (list-foldl (lambda (acc item) (cons (f item) acc)) nil lst))
))

(define foldl (lambda (f init)
  (pipe list (lambda (lst) (list-foldl f init lst) ) )
))

(define quit (pipe list (lambda (lst)
  (exit (if (nil? lst) (lambda () 0) (lambda () (car lst)) ))
)))

(define list-index (lambda (i lst)
  (if (nil? lst) (lambda ()
    nil
  ) (lambda () (if (= i 0) (lambda ()
    (car lst)
  ) (lambda ()
    (list-index (- i 1) (cdr lst))
  ))))
))

(define quoted-example (quote list
  ; notice: `(quote list example list and)` is illegal if not quoted
  (this is an (quote list example list and) (the (number is) forty-two.))
))

(define list-append (lambda (lst x)
  (if (nil? lst) (lambda ()
    (cons x nil)
  ) (lambda ()
    (cons (car lst) (list-append (cdr lst) x))
  ))
))

(define true (= 42 42))
(define false (> 42 42))

(define list-concat (lambda (dst src)
  (if (nil? dst) (lambda ()
    src
  ) (lambda ()
    (cons (car dst) (list-concat (cdr dst) src))
  ))
))

(debug (sum 1 2 3 4))
