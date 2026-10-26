/* Explicit opt-in real-speaker test. Run with a quiet generated test library. */
#define _POSIX_C_SOURCE 200809L
#include "music_player.h"
#include "em_hal_audio.h"
#include <stdio.h>
#include <time.h>
int main(void) {
    int previous=em_get_audio_vol();
    if (em_set_audio_vol(15) || music_init()) return 1;
    int failed=0;
    for (int track=0;track<2;++track) {
        if(music_play(track)) { failed=1; break; }
        struct timespec pause={0,100000000};
        for(int i=0;i<30;++i)nanosleep(&pause,NULL);
        music_snapshot s; music_get_snapshot(&s);
        printf("track=%d state=%d position=%.2f error=%s\n",track,s.state,s.position,s.error);
        if(s.state!=MUSIC_PLAYING || s.position<1)failed=1;
        music_stop();
    }
    music_shutdown();
    em_set_audio_vol(previous);
    return failed;
}
