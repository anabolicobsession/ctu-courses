#include "queue.h"

queue_t* create_queue(int capacity) {
    queue_t *q = (queue_t *)malloc(sizeof(queue_t));
    q->queue = (void **)malloc(capacity * sizeof(void *));
    q->capacity = capacity;
    q->size = 0;
    q->head = 0;
    q->tail = 0; 
    return q;
}

void delete_queue(queue_t *queue) {
    if (queue != NULL) {
        free(queue->queue);
        queue->queue = NULL;
        free(queue);
        queue = NULL;
    }
}

void realloc_queue(queue_t *q, int multiplier, char operation) {
    int new_capacity = (operation == '*') ? q->capacity * multiplier : q->capacity / multiplier;
    int smallest_capacity = q->capacity < new_capacity ? q->capacity : new_capacity;

    queue_t *tempq = create_queue(smallest_capacity);
    for (int i = 0; i < q->size; ++i) {
        tempq->queue[i] = q->queue[(q->tail + i) % q->capacity];
    }
    for (int i = 0; i < q->size; ++i) {
        q->queue[i] = tempq->queue[i];
    }

    q->queue = realloc(q->queue, new_capacity * sizeof(void *));    
    q->tail = 0;
    q->head = (q->size == new_capacity) ? 0 : q->size;
    q->capacity = new_capacity;
    delete_queue(tempq);
}

bool push_to_queue(queue_t *queue, void *data) {
    if (queue->size >= queue->capacity) {
        realloc_queue(queue, 2, '*');
    }

    queue->queue[queue->head % queue->capacity] = data;
    queue->head = (queue->head + 1) % queue->capacity;
    ++queue->size;
    return true;
}

void* pop_from_queue(queue_t *queue) {
    if (queue->size <= 0) {
        return NULL;
    }
    
    int *ptr = queue->queue[queue->tail];
    queue->tail = (queue->tail + 1) % queue->capacity;
    --queue->size;

    if (queue->size < ((float)queue->capacity / 3) - 1) {
        realloc_queue(queue, 3, '/');
    }
    return ptr;
}

void* get_from_queue(queue_t *queue, int idx) { 
    if (idx < 0 || idx >= queue->size) {
        return NULL;
    }
    return queue->queue[(queue->tail + idx) % queue->capacity];
}

int get_queue_size(queue_t *queue) {
    return queue->size;
}
