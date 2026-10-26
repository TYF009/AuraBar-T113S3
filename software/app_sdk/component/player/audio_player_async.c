#include "audio_player_async.h"
#include "em_hal_audio.h"
#include "music_player.h"
#include "osal_thread.h"
#include "osal_queue.h"
#include <stdio.h>
#include <string.h>
typedef struct { char path[512]; } notification;
static osal_queue_t queue;
static osal_thread_t worker;
static void *run(void *unused) {
    (void)unused;
    notification n;
    for (;;) {
        if(osal_queue_recv(&queue,&n,1000)!=OSAL_SUCCESS)continue;
        if(!n.path[0])break;
        if(music_notification_begin()==0) {
            em_play_audio(n.path);
            music_notification_end();
        }
    }
    return NULL;
}
void start_play_audio_async(const char *path) {
    if(!queue || !path || !*path || strlen(path)>=sizeof(((notification*)0)->path))return;
    notification n={0};
    snprintf(n.path,sizeof(n.path),"%s",path);
    if(osal_queue_send(&queue,&n,sizeof(n),0)!=OSAL_SUCCESS)
        fprintf(stderr,"Notification queue full; repeated alert skipped\n");
}
int init_async_audio_player(void) {
    if(queue)return 0;
    if(osal_queue_create(&queue,"notifications",sizeof(notification),4)!=OSAL_SUCCESS)return -1;
    if(osal_thread_create(&worker,run,NULL)!=OSAL_SUCCESS) {
        osal_queue_delete(&queue); return -1;
    }
    return 0;
}
void deinit_async_audio_player(void) {
    if(!queue)return;
    notification n;
    em_stop_play_audio();
    while(osal_queue_recv(&queue,&n,0)==OSAL_SUCCESS) {}
    memset(&n,0,sizeof(n));
    osal_queue_send(&queue,&n,sizeof(n),1000);
    osal_thread_join(&worker,NULL);
    osal_queue_delete(&queue);
}
