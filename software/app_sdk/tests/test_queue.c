#define _POSIX_C_SOURCE 200809L
#include "osal_queue.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <time.h>
static double now(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9;}
static osal_queue_t q;
static void *producer(void *unused) {
    (void)unused;
    for(int i=0;i<1000;++i)assert(osal_queue_send(&q,&i,sizeof(i),2000)==OSAL_SUCCESS);
    return NULL;
}
int main(void) {
    osal_queue_t other;
    assert(osal_queue_create(&q,"same-name",sizeof(int),4)==OSAL_SUCCESS);
    assert(osal_queue_create(&other,"same-name",sizeof(int),4)==OSAL_SUCCESS);
    int value=-1;
    double started=now();
    assert(osal_queue_recv(&q,&value,40)!=OSAL_SUCCESS);
    assert(now()-started>=0.025 && now()-started<2);
    pthread_t t; assert(!pthread_create(&t,NULL,producer,NULL));
    for(int i=0;i<1000;++i){assert(osal_queue_recv(&q,&value,2000)==OSAL_SUCCESS);assert(value==i);}
    pthread_join(t,NULL);
    assert(osal_queue_recv(&other,&value,0)!=OSAL_SUCCESS);
    assert(osal_queue_delete(&q)==OSAL_SUCCESS && !q);
    assert(osal_queue_delete(&other)==OSAL_SUCCESS);
    puts("queue: ordered concurrent delivery, timeout and instance isolation passed");
}
