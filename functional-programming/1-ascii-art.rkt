#lang racket

(require 2htdp/image)

(provide img->mat ascii-art)

(define (split-list n lst [block '()])
  (if (null? lst)
      (if (null? block) '() (list (reverse block)))
      (let ([new-block (cons (car lst) block)])
        (if (< (length new-block) n)
          (split-list n (cdr lst) new-block)
          (append (list (reverse new-block)) (split-list n (cdr lst)))
          ))))

(define transpose
  (curry apply map list))

(define (sum-blocks w h mat)
  (define (sum-list-blocks n m)
    (transpose
     (map (lambda (block) (apply map + (transpose block)))
          (map (curry split-list n) m))))
  (sum-list-blocks h (sum-list-blocks w mat)))

(define (map-matrix-rows func matrix)
  (map (curry func) matrix))

(define (map-matrix func matrix)
  (map-matrix-rows (lambda (row) (map func row)) matrix))

(define (avg-blocks w h mat)
  (map-matrix (lambda (x) (/ x (* w h))) (sum-blocks w h mat)))

(define (drop-extra rows cols mat)
      (map (lambda (x) (drop-right x cols)) (drop-right mat rows)))

(define (rgb->grayscale color)
  (+ (* 0.3 (color-red color))
     (* 0.59 (color-green color))
     (* 0.11 (color-blue color))))

(define (img->mat img)
  (split-list (image-width img) (map rgb->grayscale (image->color-list img))))

(define (ascii-art w h chars)
  (define chars-list
    (string->list chars))
  
  (define chars-len
    (length chars-list))

  (define (intensity->char i)
    (list-ref chars-list (inexact->exact (floor (/ (* chars-len (- 255 (floor i))) 256)))))

  (define (matrix->string matrix)
    (list->string (apply append (map-matrix-rows (lambda (row) (append row (list #\newline))) matrix))))

  (lambda (img)
    (if (or (< (image-width img) w) (< (image-height img) h))
        ""
        (let* ([mat (img->mat img)]
               [rows-to-drop (remainder (length mat) h)]
               [cols-to-drop (remainder (length (car mat)) w)])
          
          (matrix->string
           (map-matrix (curry intensity->char)
                       (avg-blocks w h
                                   (drop-extra rows-to-drop cols-to-drop mat)))))
        )))