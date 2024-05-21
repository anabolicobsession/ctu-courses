#lang racket

(provide levenshtein)

(define (levenshtein str1 str2)
  (define (iter s1 s2)
    (if (or (null? s1) (null? s2))
        (if (null? s1) (length s2) (length s1))
        (if (equal? (car s1) (car s2))
            (iter (cdr s1) (cdr s2))
            (+ 1 (min (iter (cdr s1) s2) (iter s1 (cdr s2)) (iter (cdr s1) (cdr s2)))))))

  (iter (string->list str1) (string->list str2)))