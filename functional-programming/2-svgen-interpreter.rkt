#lang racket

(provide execute)

(module+ test
  (require rackunit))

(define (my-assv key table)
  (last (filter (compose ((curry eqv?) key) car) table)))

(define (num-op->fun op)
  (match op
    ['+ +]
    ['- -]
    ['* *]
    ['/ /]
    ['floor floor]
    ['cos cos]
    ['sin sin]))

(define (eval-num-exp exp [vars '()])
  (match exp
    [(? number?) exp]
    [(? symbol?) (cadr (my-assv exp vars))]
    [(list-rest (? symbol? num-op) args) (apply (num-op->fun num-op) (map (lambda (arg) (eval-num-exp arg vars)) args))]))

(define (eval-arg arg [vars '()])
  (match arg
    [(? string?) arg]
    [_ (eval-num-exp arg vars)]))

(define (bool-op->fun op)
  (match op
    ['= =]
    ['< <]
    ['> >]))

(define (eval-bool-exp exp [vars '()])
  (apply (bool-op->fun (car exp)) (map (lambda (ex) (eval-num-exp ex vars)) (cdr exp))))

(define (eval-expression expr env [vars '()])
  (define (eval-svg-tag formt args)
    (apply (curry format formt) (map (lambda (a) (eval-arg a vars)) args)))
  
  (match expr
    [(list-rest 'circle args) (eval-svg-tag "<circle cx=\"~a\" cy=\"~a\" r=\"~a\" style=\"~a\"/>" args)]
    [(list-rest 'rect args) (eval-svg-tag "<rect x=\"~a\" y=\"~a\" width=\"~a\" height=\"~a\" style=\"~a\"/>" args)]
    [(list-rest 'line args) (eval-svg-tag "<line x1=\"~a\" y1=\"~a\" x2=\"~a\" y2=\"~a\" style=\"~a\"/>" args)]
    [(list 'if bool-exp exp1 exp2) (if (eval-bool-exp bool-exp vars)
                                       (eval-expression exp1 env vars)
                                       (eval-expression exp2 env vars))]
    [(list-rest 'when bool-exp exprs) (if (eval-bool-exp bool-exp vars)
                                          (apply string-append (map (lambda (ex) (eval-expression ex env vars)) exprs))
                                          "")]
    [(list-rest (? symbol? fun-id) args) (let* ([evaluated-args (map (lambda (a) (eval-arg a vars)) args)]
                                                [fun-vars  (cadr (my-assv fun-id env))]
                                                [fun-exprs (cddr (my-assv fun-id env))]
                                                [extended-vars (append vars (map list fun-vars evaluated-args))])
                                           (apply string-append (map (lambda (ex) (eval-expression ex env extended-vars)) fun-exprs)))]))

(define (definition->env-fun def)
  (match def
    [(list-rest 'define (list-rest fun-id args-id) exprs) (append (list fun-id) (list args-id) exprs)]))
  
(define (execute width height prg expr)
    (format "<svg width=\"~a\" height=\"~a\">~a</svg>" width height (eval-expression expr (map definition->env-fun prg))))

(module+ test
  (test-case "execute"
             (check-equal? (execute 400 400 '() '(line 10 20 30 40 "stroke:black;stroke-width:5")) "<svg width=\"400\" height=\"400\"><line x1=\"10\" y1=\"20\" x2=\"30\" y2=\"40\" style=\"stroke:black;stroke-width:5\"/></svg>")))