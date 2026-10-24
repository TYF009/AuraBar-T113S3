#define _POSIX_C_SOURCE 200809L
#include "task_status.h"
#include <curl/curl.h>
#include <stdio.h>
#include <time.h>
int main(void) {
    if(curl_global_init(CURL_GLOBAL_DEFAULT))return 1;
    if(task_status_init())return 1;
    for(int i=0;i<24;++i) {
        task_snapshot s;task_status_snapshot(&s);
        printf("connected=%d count=%d notice=%d\n",s.connected,s.count,task_status_take_notice());
        fflush(stdout);
        struct timespec delay={1,0};nanosleep(&delay,NULL);
    }
    task_status_shutdown();return 0;
}
