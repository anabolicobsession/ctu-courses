#lang racket
 
(provide minimum-spanning-tree (struct-out edge) graph)
 
(struct edge (u v weight) #:transparent)
 
(struct graph (nodes edges) #:transparent)
 
(define  gr (graph '(B C D E F G)
                   (list (edge 'G 'B 1)
                         (edge 'D 'E 4)
                         (edge 'E 'F 7)
                         (edge 'G 'D 5)
                         (edge 'B 'E 2)
                         (edge 'C 'F 5)
                         (edge 'D 'B 6)
                         (edge 'E 'C 4)
                         (edge 'G 'E 3))))
 
(define (min-weight g es covered)
  (define notCovered
    (filter (λ n (not (member n covered))) (graph-nodes g)))
 
  (define (isGoodEdge e)
    (define n1 (edge-u e))
    (define n2 (edge-v e))
    (or (and (member n1 covered) (not (member n2 covered))) (and (member n2 covered) (not (member n1 covered)))))
 
  (define filtered
    (filter (curry isGoodEdge) (filter (λ e (not (member e es))) (graph-edges g))))
 
  (define sorted
    (sort filtered (lambda (e1 e2) (< (edge-weight e1) (edge-weight e2)))))
 
  (car sorted))
 
(define (iter g es covered)
  (if (equal? (length covered) (length (graph-nodes g)))
      es
      (let ([e (min-weight g es covered)])
        (iter g (cons e es) (cons (if (member (edge-u e) covered) (edge-v e) (edge-u e)) covered)))))
       
 
(define (minimum-spanning-tree g) (iter g '() (list (car (graph-nodes g)))))