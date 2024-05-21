#lang racket

(provide propagate-units (struct-out pos) (struct-out neg))

(struct pos (variable) #:transparent)
(struct neg (variable) #:transparent)

(define formula1 (list (list (neg "x"))))
(define formula2 (list (list (pos "x")) (list (neg "x")) (list (pos "y")) (list (neg "y"))))

(define (negate l)
  (match l
    [(pos v) (neg v)]
    [(neg v) (pos v)]))

(define (has-unit f)
  (if (null? f)
      #f
      (if (equal? (length (car f)) 1)
          #t
          (has-unit (cdr f)))))

(define (get-unit f)
  (if (null? f)
      (error "Illegal state")
      (if (equal? (length (car f)) 1)
          (car (car f))
          (get-unit (cdr f)))))

(define (propagate-unit u f)
  (if (null? f)
      '()
      (let ([c (car f)]
            [cs (propagate-unit u (cdr f))])
        (if (or (member u c) (member (negate u) c))
            (if (member u c)
                cs
                (cons (remove (negate u) c) cs))
            (cons c cs)))))

(define (propagate-units f)
  (if (has-unit f)
      (let* ([u (get-unit f)]
             [f2 (remove (list u) f)])
        (propagate-units (propagate-unit u f2)))
      (remove-duplicates f)))