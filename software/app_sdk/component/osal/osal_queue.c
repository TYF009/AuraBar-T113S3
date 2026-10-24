#define _POSIX_C_SOURCE 200809L
#include "osal_queue.h"
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    pthread_mutex_t lock;
    pthread_cond_t readable, writable;
    unsigned char *data;
    size_t size, capacity, head, count;
} queue_t;

static struct timespec deadline(uint32_t ms) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    t.tv_sec += ms / 1000;
    t.tv_nsec += (long)(ms % 1000) * 1000000L;
    if (t.tv_nsec >= 1000000000L) { ++t.tv_sec; t.tv_nsec -= 1000000000L; }
    return t;
}
static void unlock_queue(void *arg) { pthread_mutex_unlock(arg); }

OSAL_RESULT_T osal_queue_create(osal_queue_t *out, const char *name, uint32_t size, uint32_t capacity) {
    (void)name;
    if (!out || !size || !capacity || capacity > SIZE_MAX / size) return OSAL_INVALID_PARAM;
    queue_t *q = calloc(1, sizeof(*q));
    if (!q) return OSAL_MEMORY_ERROR;
    q->data = calloc(capacity, size);
    if (!q->data) { free(q); return OSAL_MEMORY_ERROR; }
    q->size = size; q->capacity = capacity;
    pthread_condattr_t attr;
    pthread_condattr_init(&attr);
    pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->readable, &attr);
    pthread_cond_init(&q->writable, &attr);
    pthread_condattr_destroy(&attr);
    *out = q;
    return OSAL_SUCCESS;
}
OSAL_RESULT_T osal_queue_send(osal_queue_t *handle, const void *data, uint32_t len, uint32_t ms) {
    if (!handle || !*handle || !data) return OSAL_INVALID_PARAM;
    queue_t *q = *handle;
    if (len > q->size) return OSAL_INVALID_PARAM;
    struct timespec until = deadline(ms);
    OSAL_RESULT_T result = OSAL_ERROR;
    pthread_mutex_lock(&q->lock);
    pthread_cleanup_push(unlock_queue, &q->lock);
    while (q->count == q->capacity) {
        if (!ms || pthread_cond_timedwait(&q->writable, &q->lock, &until)) break;
    }
    if (q->count < q->capacity) {
        unsigned char *dst = q->data + ((q->head + q->count) % q->capacity) * q->size;
        memset(dst, 0, q->size);
        memcpy(dst, data, len);
        ++q->count;
        pthread_cond_signal(&q->readable);
        result = OSAL_SUCCESS;
    }
    pthread_cleanup_pop(1);
    return result;
}
OSAL_RESULT_T osal_queue_recv(osal_queue_t *handle, void *data, uint32_t ms) {
    if (!handle || !*handle || !data) return OSAL_INVALID_PARAM;
    queue_t *q = *handle;
    struct timespec until = deadline(ms);
    OSAL_RESULT_T result = OSAL_ERROR;
    pthread_mutex_lock(&q->lock);
    pthread_cleanup_push(unlock_queue, &q->lock);
    while (!q->count) {
        if (!ms || pthread_cond_timedwait(&q->readable, &q->lock, &until)) break;
    }
    if (q->count) {
        memcpy(data, q->data + q->head * q->size, q->size);
        q->head = (q->head + 1) % q->capacity;
        --q->count;
        pthread_cond_signal(&q->writable);
        result = OSAL_SUCCESS;
    }
    pthread_cleanup_pop(1);
    return result;
}
/* Call only after all users have been joined. */
OSAL_RESULT_T osal_queue_delete(osal_queue_t *handle) {
    if (!handle || !*handle) return OSAL_INVALID_PARAM;
    queue_t *q = *handle;
    pthread_cond_destroy(&q->readable);
    pthread_cond_destroy(&q->writable);
    pthread_mutex_destroy(&q->lock);
    free(q->data); free(q); *handle = NULL;
    return OSAL_SUCCESS;
}
