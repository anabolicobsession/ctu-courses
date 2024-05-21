#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>

#include "linked_list.h"

#define RET_SUCCESS 0
#define RET_ERROR 1
#define RET_THREAD_ERROR -1

#define N_THREADS_DEFAULT 1
#define N_THREADS_MIN 1
#define N_THREADS_DEFAULT_MAX 32

pthread_t tidprod, tidcons[N_THREADS_DEFAULT_MAX];
pthread_mutex_t mutex_queue, mutex_print;
sem_t sem_empty, sem_full;
linked_list_t queue;
unsigned int n_cons_threads = N_THREADS_DEFAULT;

int get_thread_num() {
    pthread_t tid = pthread_self();
    for (int i = 0; i < n_cons_threads; i++) {
        if (tidcons[i] == tid) return i + 1;
    }
    
    return RET_THREAD_ERROR;
}

void *producer() {
    int *ret = malloc(sizeof(int *));
    assert(ret != NULL);

    int x;
    char *text;
    while ((*ret = scanf("%d %ms", &x, &text)) == 2) {
        if (x < 0 || strlen(text) == 0) {
            *ret = RET_ERROR;
            break;
        }

        pthread_mutex_lock(&mutex_queue);
        linked_list_push(&queue, x, text);
        pthread_mutex_unlock(&mutex_queue);

        sem_post(&sem_full);
    }

    pthread_exit((void *)ret);
}

void *consumer() {
    while(true) {
        sem_wait(&sem_full);

        pthread_mutex_lock(&mutex_queue);
        if (queue.size == 0) {
            // producer can wake up consumers ONLY if there is something to consume,
            // otherwise it means that stdout was terminated
            pthread_mutex_unlock(&mutex_queue);
            break;
        }
        node_t *node = linked_list_pop(&queue);
        pthread_mutex_unlock(&mutex_queue);

        pthread_mutex_lock(&mutex_print);
        printf("Thread %d:", get_thread_num());
        for (int i = 0; i < node->x; i++) {
            printf(" %s", node->text);
        }
        printf("\n");
        pthread_mutex_unlock(&mutex_print);

        // actually it's not in linked list, so mutex is no needed
        linked_list_clear_node(node);

        sem_post(&sem_empty);
    }

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    int ret = RET_SUCCESS;

    if (argc > 1) {
        int argv1 = atoi(argv[1]);
        if (argv1 >= N_THREADS_MIN && argv1 <= sysconf(_SC_NPROCESSORS_ONLN)) {
            n_cons_threads = argv1;
        } else {
            return RET_ERROR;
        }
    }

    linked_list_init(&queue);
    assert(sem_init(&sem_empty, 0, 0) == 0);
    assert(sem_init(&sem_full, 0, 0) == 0);
    assert(pthread_mutex_init(&mutex_queue, NULL) == 0);
    assert(pthread_mutex_init(&mutex_print, NULL) == 0);

    assert(pthread_create(&tidprod, NULL, producer, NULL) == 0);
    for (int i = 0; i < n_cons_threads; i++) {
        assert(pthread_create(&tidcons[i], NULL, consumer, NULL) == 0);
    }
    
    int *prod_status = NULL;
    assert(pthread_join(tidprod, (void *)&prod_status) == 0);
    if (*prod_status != EOF) ret = RET_ERROR;
    free(prod_status);
    
    // wake up all the consumers and wait for joining
    for (int i = 0; i < n_cons_threads; i++) {
        sem_post(&sem_full);
    }
    for (int i = 0; i < n_cons_threads; i++) {
        assert(pthread_join(tidcons[i], NULL) == 0); 
    }

    assert(pthread_mutex_destroy(&mutex_queue) == 0);
    assert(pthread_mutex_destroy(&mutex_print) == 0);
    assert(sem_destroy(&sem_empty) == 0);
    assert(sem_destroy(&sem_full) == 0);
    linked_list_clear(&queue);

    return ret;
}
