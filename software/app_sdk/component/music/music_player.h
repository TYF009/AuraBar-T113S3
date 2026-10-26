#ifndef AURABAR_MUSIC_PLAYER_H
#define AURABAR_MUSIC_PLAYER_H
#include <stdbool.h>
#define MUSIC_MAX_TRACKS 64
typedef enum { MUSIC_STOPPED, MUSIC_BUFFERING, MUSIC_PLAYING, MUSIC_PAUSED, MUSIC_ERROR } music_state;
typedef struct {
    music_state state;
    int index, count;
    double position, duration;
    bool repeat;
    bool favorite;
    unsigned sleep_seconds;
    char title[128], error[128];
} music_snapshot;
int music_init(void);
void music_shutdown(void);
int music_reload(void); /* Stop playback before rescanning. */
void music_get_snapshot(music_snapshot *out);
int music_track_title(int index, char *dst, unsigned size);
int music_play(int index);
void music_stop(void);
void music_toggle_pause(void);
void music_next(int delta);
void music_seek(double seconds); /* Local files only; duration=0 denotes a live stream. */
void music_set_repeat(bool repeat);
void music_toggle_favorite(void);
void music_sleep_after(unsigned minutes); /* 0 cancels; CLOCK_MONOTONIC. */
int music_notification_begin(void);
void music_notification_end(void);
#endif
