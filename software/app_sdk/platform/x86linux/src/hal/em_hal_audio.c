#define _POSIX_C_SOURCE 200809L
#include "em_hal_audio.h"
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <spawn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
extern char **environ;
static pthread_mutex_t child_lock=PTHREAD_MUTEX_INITIALIZER;
static pid_t owned_child=-1;
static int wait_child(pid_t pid) {
    int status=0;
    while (waitpid(pid,&status,0)<0) { if(errno!=EINTR)return -1; }
    return WIFEXITED(status)&&WEXITSTATUS(status)==0?0:-1;
}
void em_hal_audio_output_enable(int enabled) {
#ifndef SIMULATOR_LINUX
    FILE *f=fopen("/sys/class/gpio/gpio34/value","w");
    if(f) { fputs(enabled?"0":"1",f); fclose(f); }
#else
    (void)enabled;
#endif
}
int em_get_audio_vol(void) {
#ifdef SIMULATOR_LINUX
    return 50;
#else
    FILE *fp=popen("amixer -D hw:audiocodec cget name='DAC volume'","r");
    if(!fp)return 0;
    char line[256]; long value=0;
    while(fgets(line,sizeof(line),fp)) {
        char *p=strstr(line,"values=");
        if(!p)continue;
        p+=7;
        while(*p) {
            char *end;
            long n=strtol(p,&end,10);
            if(end==p)break;
            if(n>value)value=n;
            p=*end==','?end+1:end;
            if(*end!=',')break;
        }
    }
    pclose(fp);
    if(value<0)value=0;
    if(value>200)value=200;
    return (int)value/2;
#endif
}
int em_set_audio_vol(int volume) {
    if(volume<0)volume=0;
    if(volume>100)volume=100;
#ifdef SIMULATOR_LINUX
    (void)volume; return 0;
#else
    char value[24]; snprintf(value,sizeof(value),"0,%d",volume*2);
    char *argv[]={"amixer","-q","-D","hw:audiocodec","cset","name=DAC volume",value,NULL};
    pid_t pid=-1;
    if(posix_spawnp(&pid,"amixer",NULL,NULL,argv,environ))return -1;
    return wait_child(pid);
#endif
}
void em_stop_play_audio(void) {
    pthread_mutex_lock(&child_lock);
    if(owned_child>0)kill(owned_child,SIGTERM);
    pthread_mutex_unlock(&child_lock);
}
/* Synchronous; called on the notification worker, never on the LVGL thread. */
int em_play_audio(const char *path) {
    if(!path || !*path)return -1;
    char *argv[]={"aplay","-q","--",(char*)path,NULL};
    pid_t pid=-1;
    em_hal_audio_output_enable(1);
    pthread_mutex_lock(&child_lock);
    int ret=posix_spawnp(&pid,"aplay",NULL,NULL,argv,environ);
    if(!ret)owned_child=pid;
    pthread_mutex_unlock(&child_lock);
    if(!ret)ret=wait_child(pid);
    pthread_mutex_lock(&child_lock);
    if(owned_child==pid)owned_child=-1;
    pthread_mutex_unlock(&child_lock);
    em_hal_audio_output_enable(0);
    return ret?-1:0;
}
