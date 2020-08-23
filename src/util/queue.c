#include <assert.h>

#include "queue.h"
#include "util.h"

#ifdef NDEBUG
#  define PTRY(pcall) (void) pcall
#else // !defined NDEBUG
#  define PTRY(pcall) 
#endif // !defined NDEBUG

int pthread_queue_init(pthread_queue_t *queue) {
    PTRY(pthread_mutex_init(&queue->mutex, NULL));
    PTRY(pthread_cond_init(&queue->cond, NULL));
    queue->front = queue->back = NULL;
    return 0;
}

int pthread_queue_destroy(pthread_queue_t *queue) {
    int result;
    if ((result = pthread_mutex_trylock(&queue->mutex)) == 0) {
        if (queue->front || queue->back) {
        }
        PTRY(pthread_cond_destroy(&queue->cond));
        PTRY(pthread_mutex_unlock(&queue->mutex));
        PTRY(pthread_mutex_destroy(&queue->mutex));
    }
    else {
    }
    return result;
}

bool pthread_queue_is_empty(pthread_queue_t *queue) {
    PTRY(pthread_mutex_lock(&queue->mutex));
    bool result = (queue->front == NULL);
    PTRY(pthread_mutex_unlock(&queue->mutex));
    return result;
}

bool pthread_queue_has_been_empty(pthread_queue_t *queue, clock_t ms) {
    PTRY(pthread_mutex_lock(&queue->mutex));
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += (ms % 1000) * 1000000;
    ts.tv_sec += (ms / 1000) + (ts.tv_nsec / 1000000000L);
    ts.tv_nsec %= 1000000000L;
    
    int timed_out = 0;
    bool empty;
    while ((empty = (queue->front == NULL)) && !timed_out) {
        timed_out = pthread_cond_timedwait(&queue->cond, &queue->mutex, &ts);
    }

    PTRY(pthread_mutex_unlock(&queue->mutex));
    return empty;
}

int pthread_queue_push_back(pthread_queue_t *queue, void *data) {
    queue_element_t *cons = NEW(queue_element_t);
    cons->head = data;
    cons->tail = NULL;
    PTRY(pthread_mutex_lock(&queue->mutex));
    if (queue->back == NULL) {
        assert(queue->front == NULL);
        queue->front = queue->back = cons;
    }
    else {
        queue->back->tail = (struct queue_element_t*)cons;
        queue->back = cons;
    }
    PTRY(pthread_cond_broadcast(&queue->cond));
    PTRY(pthread_mutex_unlock(&queue->mutex));
    return 0;
}

void *pthread_queue_pop_front(pthread_queue_t *queue) {
    queue_element_t *cons;
    void *data;
    PTRY(pthread_mutex_lock(&queue->mutex));
    while (queue->front == NULL) {
        PTRY(pthread_cond_wait(&queue->cond, &queue->mutex));	
    }
    cons = queue->front;
    queue->front = (queue_element_t*)cons->tail;
    if (queue->front == NULL) {
        queue->back = NULL;
    }
    PTRY(pthread_mutex_unlock(&queue->mutex));
    data = cons->head;
    free(cons);
    return data;
}
