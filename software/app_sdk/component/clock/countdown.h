#ifndef AURABAR_COUNTDOWN_H
#define AURABAR_COUNTDOWN_H
#include <stdbool.h>
#include <stdint.h>
typedef struct { uint64_t remaining_ms, last_ms; bool running; } countdown;
static inline void countdown_start(countdown *t,uint64_t duration,uint64_t now) {
    t->remaining_ms=duration; t->last_ms=now; t->running=duration!=0;
}
static inline uint64_t countdown_update(countdown *t,uint64_t now) {
    uint64_t elapsed=now>=t->last_ms?now-t->last_ms:0;
    if(t->running) {
        t->remaining_ms=elapsed>=t->remaining_ms?0:t->remaining_ms-elapsed;
        if(!t->remaining_ms)t->running=false;
    }
    t->last_ms=now;
    return t->remaining_ms;
}
static inline void countdown_running(countdown *t,bool running,uint64_t now) {
    countdown_update(t,now); t->running=running && t->remaining_ms!=0;
}
#endif
