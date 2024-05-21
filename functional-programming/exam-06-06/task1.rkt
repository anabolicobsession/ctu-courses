#lang racket

(provide reshape)

(define m '((1 2 3)
            (4 5 6)))

(define (rollout mtx)
  (apply append mtx))

(define (split n lst [sub-lst '()])
  (if (null? lst)
      (list (reverse sub-lst))
      (if (eqv? n (length sub-lst))
          (cons (reverse sub-lst) (split n lst))
          (split n (cdr lst) (cons (car lst) sub-lst)))))

(define (reshape mtx r c)
  (let ([lst (rollout mtx)])
    (if (eqv? (length lst) (* r c))
        (split c (rollout mtx))
        (error "invalid shape"))))
  