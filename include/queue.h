#ifndef PTHREAD_QUEUE_H
#define PTHREAD_QUEUE_H

#include <pthread.h>
#include <stdbool.h>
/*
 * Interfejs obslugi wielowatkowosci.
 *
 * Autor: Panicz Maciej Godek <maciej.godek@kayon.pl>, listopad 2018
 * Program jest wlasnoscia firmy Kayon. Wszystkie prawa zastrzezone.
 */

typedef struct {
    void *head;
    struct queue_element_t *tail;
} queue_element_t;

typedef struct {
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    queue_element_t *front;
    queue_element_t *back;
    queue_element_t *current;
} pthread_queue_t;

extern int pthread_queue_init(pthread_queue_t *queue);
extern int pthread_queue_destroy(pthread_queue_t *queue);
extern bool pthread_queue_is_empty(pthread_queue_t *queue);
extern bool pthread_queue_has_been_empty(pthread_queue_t *queue, clock_t ms);
extern int pthread_queue_push_back(pthread_queue_t *queue, void *data);
extern void *pthread_queue_pop_front(pthread_queue_t *queue);

#endif // PTHREAD_QUEUE_H
