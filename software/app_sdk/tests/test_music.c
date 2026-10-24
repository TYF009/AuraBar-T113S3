#define _POSIX_C_SOURCE 200809L
#include "music_player.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
static void wait_ms(long ms){struct timespec t={ms/1000,(ms%1000)*1000000L};nanosleep(&t,NULL);}
static music_snapshot snap(void){music_snapshot s;music_get_snapshot(&s);return s;}
static void playing(void){
    for(int i=0;i<100;++i){music_snapshot s=snap();if(s.state==MUSIC_PLAYING)return;if(s.state==MUSIC_ERROR){fprintf(stderr,"%s\n",s.error);assert(0);}wait_ms(50);}
    assert(!"player did not start");
}
int main(void){
    setenv("AURABAR_AUDIO_DEVICE","null",1);
    assert(!music_init());
    music_snapshot s=snap(); assert(s.count>=4);
    assert(music_play(-1)==-1 && music_play(s.count)==-1);
    assert(!music_play(0)); playing(); wait_ms(150);
    music_toggle_pause(); wait_ms(180);
    s=snap(); assert(s.state==MUSIC_PAUSED);
    double position=s.position; wait_ms(200); assert(fabs(snap().position-position)<0.02);
    music_seek(4); music_toggle_pause(); playing(); wait_ms(100); assert(snap().position>=4);
    assert(!music_notification_begin()); wait_ms(100); assert(snap().state==MUSIC_PAUSED);
    music_notification_end(); playing();
    music_set_repeat(true); assert(snap().repeat);
    bool favorite=snap().favorite;
    music_toggle_favorite(); assert(snap().favorite!=favorite);
    music_toggle_favorite(); assert(snap().favorite==favorite);
    music_sleep_after(15); assert(snap().sleep_seconds>850); music_sleep_after(0);
    assert(!snap().sleep_seconds);
    if(s.count>=5){music_play(1);playing();assert(snap().duration>10);music_seek(6);wait_ms(150);assert(snap().position>=6);}
    music_next(1); playing();
    for(int i=0;i<s.count;++i) {
        char name[128]; music_track_title(i,name,sizeof(name));
        if(!strcmp(name,"HTTP test")) {
            music_play(i); playing(); wait_ms(100);
            assert(snap().duration==0 && snap().position>0);
        }
        if(!strcmp(name,"HTTP failure")) {
            music_play(i);
            for(int n=0;n<100 && snap().state!=MUSIC_ERROR;++n)wait_ms(30);
            assert(snap().state==MUSIC_ERROR);
        }
    }
    music_stop(); wait_ms(150); assert(snap().state==MUSIC_STOPPED);
    assert(!music_reload());
    music_shutdown();
    puts("music: available local/HTTP sources, pause, seek, notifications and sleep control passed");
}
