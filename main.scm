(define bare-string& 'Hello, liftA42!')

(define write-string (lambda (str&)
  (str& (foldr (lambda (acc item) (write 1 item)) nil))
))

(write-string bare-string&)
(write 1 10)

(main& (lambda (argv&)
  (argv& (lambda (argv1 argv2)
    (write-string argv1)
    (write 1 10)
    (write-string argv2)
    (write 1 10)
  ))
))
