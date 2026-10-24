#define _POSIX_C_SOURCE 200809L
#include "task_status.h"
#include <curl/curl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BODY_LIMIT 32768
static struct {
    pthread_mutex_t lock;
    pthread_t thread;
    int started,quit;
    char file[512],url[768],authorization[192];
    task_model model;
} monitor={.lock=PTHREAD_MUTEX_INITIALIZER};
static double monotonic(void) { struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return t.tv_sec+t.tv_nsec/1e9; }
typedef struct { char text[BODY_LIMIT+1];size_t size; } response;
static size_t receive(void *data,size_t size,size_t count,void *context) {
    response *r=context;
    if(size && count>BODY_LIMIT/size)return 0;
    size_t n=size*count;
    if(n>BODY_LIMIT-r->size)return 0;
    memcpy(r->text+r->size,data,n);r->size+=n;r->text[r->size]=0;return n;
}
static int read_snapshot(response *r) {
    if(monitor.file[0]) {
        FILE *f=fopen(monitor.file,"rb");if(!f)return -1;
        r->size=fread(r->text,1,BODY_LIMIT+1,f);
        int failed=ferror(f)||r->size>BODY_LIMIT;
        fclose(f);if(failed)return -1;r->text[r->size]=0;return 0;
    }
    CURL *c=curl_easy_init();if(!c)return -1;
    struct curl_slist *headers=curl_slist_append(NULL,monitor.authorization);
    if(!headers) { curl_easy_cleanup(c);return -1; }
    curl_easy_setopt(c,CURLOPT_URL,monitor.url);
    curl_easy_setopt(c,CURLOPT_HTTPHEADER,headers);
    curl_easy_setopt(c,CURLOPT_NOSIGNAL,1L);
    curl_easy_setopt(c,CURLOPT_CONNECTTIMEOUT,2L);
    curl_easy_setopt(c,CURLOPT_TIMEOUT,3L);
    curl_easy_setopt(c,CURLOPT_SSL_VERIFYPEER,1L);
    curl_easy_setopt(c,CURLOPT_SSL_VERIFYHOST,2L);
    curl_easy_setopt(c,CURLOPT_PROXY,"");
#if LIBCURL_VERSION_NUM >= 0x075500
    curl_easy_setopt(c,CURLOPT_PROTOCOLS_STR,"http,https");
#else
    curl_easy_setopt(c,CURLOPT_PROTOCOLS,CURLPROTO_HTTP|CURLPROTO_HTTPS);
#endif
    const char *ca=getenv("AURABAR_CA_BUNDLE");if(ca&&*ca)curl_easy_setopt(c,CURLOPT_CAINFO,ca);
    curl_easy_setopt(c,CURLOPT_WRITEFUNCTION,receive);
    curl_easy_setopt(c,CURLOPT_WRITEDATA,r);
    CURLcode result=curl_easy_perform(c);
    long status=0;curl_easy_getinfo(c,CURLINFO_RESPONSE_CODE,&status);
    curl_slist_free_all(headers);curl_easy_cleanup(c);
    return result==CURLE_OK&&status==200?0:-1;
}
static void *worker(void *unused) {
    (void)unused;
    response *r=malloc(sizeof(*r));if(!r)return NULL;
    for(;;) {
        pthread_mutex_lock(&monitor.lock);int quit=monitor.quit;pthread_mutex_unlock(&monitor.lock);
        if(quit)break;
        memset(r,0,sizeof(*r));
        int result=read_snapshot(r);
        pthread_mutex_lock(&monitor.lock);
        if(!result && memchr(r->text,0,r->size)==NULL)
            result=task_model_accept(&monitor.model,r->text,monotonic(),monitor.file[0]!=0);
        else result=-1;
        if(result && !monitor.model.view.connected)
            snprintf(monitor.model.view.message,sizeof(monitor.model.view.message),"Waiting for companion - check link and pairing");
        task_model_age(&monitor.model,monotonic());
        pthread_mutex_unlock(&monitor.lock);
        for(int i=0;i<10;++i) {
            struct timespec delay={0,100000000};nanosleep(&delay,NULL);
            pthread_mutex_lock(&monitor.lock);quit=monitor.quit;pthread_mutex_unlock(&monitor.lock);
            if(quit)break;
        }
    }
    free(r);return NULL;
}
int task_status_init(void) {
    if(monitor.started)return 0;
    const char *file=getenv("AURABAR_TASK_FILE"),*url=getenv("AURABAR_TASK_URL");
    snprintf(monitor.model.view.message,sizeof(monitor.model.view.message),"Connect the desktop companion to receive tasks");
    if((!file||!*file)&&(!url||!*url))return 0;
    if(file&&*file) {
        if(strlen(file)>=sizeof(monitor.file))return -1;
        snprintf(monitor.file,sizeof(monitor.file),"%s",file);
    } else {
        if(strlen(url)>=sizeof(monitor.url)|| (strncmp(url,"http://",7)&&strncmp(url,"https://",8)))return -1;
        const char *path=getenv("AURABAR_TASK_TOKEN_FILE");if(!path)return -1;
        FILE *f=fopen(path,"r");if(!f)return -1;
        char token[130]={0};size_t n=fread(token,1,sizeof(token)-1,f);int extra=fgetc(f);fclose(f);
        while(n&&(token[n-1]=='\r'||token[n-1]=='\n'))token[--n]=0;
        if(extra!=EOF||n<32||n>128)return -1;
        for(size_t i=0;i<n;++i)if(!((token[i]>='a'&&token[i]<='z')||(token[i]>='A'&&token[i]<='Z')||(token[i]>='0'&&token[i]<='9')))return -1;
        snprintf(monitor.authorization,sizeof(monitor.authorization),"Authorization: Bearer %s",token);
        memset(token,0,sizeof(token));snprintf(monitor.url,sizeof(monitor.url),"%s",url);
    }
    monitor.model.view.enabled=1;monitor.quit=0;
    if(pthread_create(&monitor.thread,NULL,worker,NULL))return -1;
    monitor.started=1;return 0;
}
void task_status_shutdown(void) {
    if(!monitor.started)return;
    pthread_mutex_lock(&monitor.lock);monitor.quit=1;pthread_mutex_unlock(&monitor.lock);
    pthread_join(monitor.thread,NULL);monitor.started=0;
}
void task_status_snapshot(task_snapshot *out) {
    pthread_mutex_lock(&monitor.lock);*out=monitor.model.view;pthread_mutex_unlock(&monitor.lock);
}
int task_status_take_notice(void) {
    pthread_mutex_lock(&monitor.lock);int pending=monitor.model.pending_notice;monitor.model.pending_notice=0;pthread_mutex_unlock(&monitor.lock);return pending;
}
