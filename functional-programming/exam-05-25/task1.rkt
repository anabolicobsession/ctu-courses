#lang racket

(provide eval state)

(struct state (x y dir) #:transparent)

(define test-state (state 0 0 'north))

(define (make-move s)
  (match s
    [(state x y 'north) (state x (+ y 1) 'north)]
    [(state x y 'south) (state x (- y 1) 'south)]
    [(state x y 'east) (state (+ x 1) y 'east)]
    [(state x y 'west) (state (- x 1) y 'west)]))

(define (rotate-to dir to)
  (define (left? to)
    (eq? 'left to))
  
  (match dir
    ['north (if (left? to) 'west 'east)]
    ['south (if (left? to) 'east 'west)] 
    ['east (if (left? to) 'north 'south)] 
    ['west (if (left? to) 'south 'north)]))

(define (eval-move s move)
  (match move
    ['move (make-move s)]
    [to (state (state-x s) (state-y s) (rotate-to (state-dir s) to))]))

(define (eval prg st)
  (foldl (λ (m s) (eval-move s m)) st prg))
