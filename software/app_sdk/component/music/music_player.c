#define _POSIX_C_SOURCE 200809L
#define _FILE_OFFSET_BITS 64
#define MINIMP3_ONLY_MP3
#define MINIMP3_NO_STDIO
#define MINIMP3_IMPLEMENTATION
#include "minimp3_ex.h"
#include "music_player.h"
#include "pcm_output.h"
#include "file_save.h"
#include <curl/curl.h>
#include <dirent.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#ifndef PROJECT_MUSIC_URL
#define PROJECT_MUSIC_URL "/usr/res/music/library"
#endif
typedef struct { char source[512], title[128]; } track;
typedef struct { uint32_t magic, count; char source[MUSIC_MAX_TRACKS][512]; } favorites;
static favorites saved;
static int is_favorite(const char *source) {
    for (unsigned i=0;i<saved.count;++i) if(!strcmp(saved.source[i],source))return 1;
    return 0;
}
static struct {
    pthread_mutex_t lock;
    pthread_cond_t changed;
    pthread_t worker;
    int initialized, quit, wanted, paused, notification, output_active;
    unsigned generation;
    double seek_to, sleep_at;
    track tracks[MUSIC_MAX_TRACKS];
    music_snapshot view;
} player;
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC,&t); return t.tv_sec+t.tv_nsec/1e9; }
static void close_output(pcm_output *pcm) {
    pcm_output_close(pcm);
    pthread_mutex_lock(&player.lock);
    player.output_active=0;
    pthread_cond_broadcast(&player.changed);
    pthread_mutex_unlock(&player.lock);
}
static int control(pcm_output *pcm, unsigned generation) {
    for (;;) {
        pthread_mutex_lock(&player.lock);
        if (player.sleep_at && now()>=player.sleep_at) {
            player.sleep_at=0; player.wanted=0; ++player.generation;
            player.view.state=MUSIC_STOPPED;
        }
        if (player.quit || !player.wanted || generation!=player.generation) {
            pthread_mutex_unlock(&player.lock); return -1;
        }
        if (!player.paused && !player.notification) {
            pthread_mutex_unlock(&player.lock); return 0;
        }
        player.view.state=MUSIC_PAUSED;
        pthread_mutex_unlock(&player.lock);
        close_output(pcm);
        pthread_mutex_lock(&player.lock);
        struct timespec t;
        clock_gettime(CLOCK_MONOTONIC,&t); t.tv_nsec+=100000000;
        if (t.tv_nsec>=1000000000) { ++t.tv_sec; t.tv_nsec-=1000000000; }
        pthread_cond_timedwait(&player.changed,&player.lock,&t);
        pthread_mutex_unlock(&player.lock);
    }
}
static int output(pcm_output *pcm, const int16_t *samples, size_t frames, unsigned rate, unsigned channels, unsigned generation) {
    for (;;) {
        if (control(pcm,generation)) return -1;
        pthread_mutex_lock(&player.lock);
        if (!player.paused && !player.notification && player.wanted && generation==player.generation)
            break;
        pthread_mutex_unlock(&player.lock);
    }
    player.output_active=1; /* Reserve before opening, so notifications wait. */
    pthread_mutex_unlock(&player.lock);
    if (pcm_output_open(pcm,rate,channels) || pcm_output_write(pcm,samples,frames)) {
        close_output(pcm); return -2;
    }
    pthread_mutex_lock(&player.lock);
    if (generation==player.generation) {
        player.view.state=MUSIC_PLAYING;
        player.view.position+=(double)frames/rate;
    }
    pthread_mutex_unlock(&player.lock);
    return 0;
}
static void duration(double seconds) {
    pthread_mutex_lock(&player.lock); player.view.duration=seconds; pthread_mutex_unlock(&player.lock);
}
static double take_seek(unsigned generation) {
    pthread_mutex_lock(&player.lock);
    double target=player.generation==generation?player.seek_to:-1;
    player.seek_to=-1;
    if (target>=0) player.view.position=target;
    pthread_mutex_unlock(&player.lock);
    return target;
}
static unsigned le16(const unsigned char *b) { return b[0]|(unsigned)b[1]<<8; }
static unsigned le32(const unsigned char *b) { return le16(b)|le16(b+2)<<16; }
static int play_wav(const char *path, unsigned generation, pcm_output *pcm) {
    FILE *f=fopen(path,"rb");
    if (!f) return -2;
    unsigned char h[12], chunk[8], fmt[16];
    unsigned rate=0, channels=0, bytes=0;
    off_t start=0;
    int ret=-2;
    struct stat st;
    if (fstat(fileno(f),&st) || fread(h,1,12,f)!=12 || memcmp(h,"RIFF",4) || memcmp(h+8,"WAVE",4)) goto done;
    while (fread(chunk,1,8,f)==8) {
        unsigned size=le32(chunk+4);
        off_t begin=ftello(f);
        if (begin<0 || (uint64_t)begin+size>(uint64_t)st.st_size) goto done;
        if (!memcmp(chunk,"fmt ",4)) {
            if (size<16 || fread(fmt,1,16,f)!=16 || le16(fmt)!=1 || le16(fmt+14)!=16) goto done;
            channels=le16(fmt+2); rate=le32(fmt+4);
            if ((channels!=1 && channels!=2) || rate<8000 || rate>192000 || le16(fmt+12)!=channels*2) goto done;
        } else if (!memcmp(chunk,"data",4)) {
            start=begin; bytes=size;
            if (rate && channels) break;
        }
        if (fseeko(f,begin+size+(size&1),SEEK_SET)) goto done;
    }
    if (!start || !bytes || !rate || !channels || bytes%(channels*2)) goto done;
    duration((double)bytes/(rate*channels*2));
    if (fseeko(f,start,SEEK_SET)) goto done;
    int16_t samples[2048];
    while (ftello(f)<start+bytes) {
        if (control(pcm,generation)) { ret=-1; goto done; }
        double seek=take_seek(generation);
        if (seek>=0) {
            uint64_t frame=(uint64_t)(seek*rate);
            if (frame>bytes/(channels*2)) frame=bytes/(channels*2);
            if (fseeko(f,start+(off_t)(frame*channels*2),SEEK_SET)) goto done;
            pcm_output_close(pcm);
        }
        size_t remaining=(size_t)(start+bytes-ftello(f));
        size_t n=remaining<sizeof(samples)?remaining:sizeof(samples);
        n=fread(samples,1,n,f);
        if (!n) { ret=ferror(f)?-2:0; goto done; }
        ret=output(pcm,samples,n/(channels*2),rate,channels,generation);
        if (ret) goto done;
    }
    ret=0;
done:
    fclose(f); return ret;
}
static size_t file_read(void *dst,size_t size,void *f) { return fread(dst,1,size,f); }
static int file_seek(uint64_t pos,void *f) { return pos>INT64_MAX?-1:fseeko(f,(off_t)pos,SEEK_SET); }
static int play_mp3(const char *path,unsigned generation,pcm_output *pcm) {
    FILE *f=fopen(path,"rb");
    if (!f) return -2;
    mp3dec_io_t io={file_read,f,file_seek,f};
    mp3dec_ex_t *dec=calloc(1,sizeof(*dec));
    if (!dec) { fclose(f); return -2; }
    int ret=-2;
    if (mp3dec_ex_open_cb(dec,&io,MP3D_SEEK_TO_SAMPLE)) goto done;
    if (!dec->info.channels || !dec->info.hz) goto done;
    duration((double)dec->samples/(dec->info.hz*dec->info.channels));
    int16_t samples[2304];
    for (;;) {
        if (control(pcm,generation)) { ret=-1; break; }
        double seek=take_seek(generation);
        if (seek>=0) {
            if (mp3dec_ex_seek(dec,(uint64_t)(seek*dec->info.hz)*dec->info.channels)) { ret=-2; break; }
            pcm_output_close(pcm);
        }
        size_t n=mp3dec_ex_read(dec,samples,sizeof(samples)/sizeof(*samples));
        if (!n) { ret=dec->last_error?-2:0; break; }
        ret=output(pcm,samples,n/dec->info.channels,dec->info.hz,dec->info.channels,generation);
        if (ret) break;
    }
done:
    mp3dec_ex_close(dec); free(dec); fclose(f); return ret;
}
typedef struct {
    mp3dec_t decoder;
    unsigned char input[32768];
    size_t used, skipped;
    int16_t samples[MINIMP3_MAX_SAMPLES_PER_FRAME];
    unsigned generation;
    pcm_output *pcm;
    int failed, decoded;
} stream_ctx;
static int decode_stream(stream_ctx *s, int final) {
    while (s->used >= (final?4:16384)) {
        mp3dec_frame_info_t info;
        int n=mp3dec_decode_frame(&s->decoder,s->input,(int)s->used,s->samples,&info);
        size_t consumed=info.frame_bytes>0?(size_t)info.frame_bytes:s->used-3;
        if (n) {
            s->decoded=1;
            int ret=output(s->pcm,s->samples,(size_t)n,info.hz,info.channels,s->generation);
            if (ret) { s->failed=ret; return -1; }
        } else {
            s->skipped+=consumed;
            if (!s->decoded && s->skipped>1024*1024) { s->failed=-2; return -1; }
        }
        if (!consumed || consumed>s->used) break;
        memmove(s->input,s->input+consumed,s->used-consumed);
        s->used-=consumed;
    }
    return 0;
}
static size_t stream_write(void *data,size_t size,size_t count,void *ctx) {
    stream_ctx *s=ctx;
    if (size && count>SIZE_MAX/size) return 0;
    size_t total=size*count, off=0;
    while (off<total) {
        if (control(s->pcm,s->generation)) { s->failed=-1; return 0; }
        size_t n=sizeof(s->input)-s->used;
        if (n>total-off) n=total-off;
        memcpy(s->input+s->used,(char*)data+off,n);
        s->used+=n; off+=n;
        if (decode_stream(s,0)) return 0;
    }
    return total;
}
static int stream_progress(void *ctx,curl_off_t a,curl_off_t b,curl_off_t c,curl_off_t d) {
    (void)a; (void)b; (void)c; (void)d;
    stream_ctx *s=ctx;
    return control(s->pcm,s->generation)!=0;
}
static int play_stream(const char *url,unsigned generation,pcm_output *pcm) {
    stream_ctx *s=calloc(1,sizeof(*s));
    CURL *curl=curl_easy_init();
    if (!s || !curl) { free(s); if(curl)curl_easy_cleanup(curl); return -2; }
    s->generation=generation; s->pcm=pcm;
    mp3dec_init(&s->decoder);
    curl_easy_setopt(curl,CURLOPT_URL,url);
    curl_easy_setopt(curl,CURLOPT_PROTOCOLS,(long)(CURLPROTO_HTTP|CURLPROTO_HTTPS));
    curl_easy_setopt(curl,CURLOPT_REDIR_PROTOCOLS,(long)(CURLPROTO_HTTP|CURLPROTO_HTTPS));
    curl_easy_setopt(curl,CURLOPT_FOLLOWLOCATION,1L);
    curl_easy_setopt(curl,CURLOPT_MAXREDIRS,3L);
    curl_easy_setopt(curl,CURLOPT_FAILONERROR,1L);
    curl_easy_setopt(curl,CURLOPT_NOSIGNAL,1L);
    curl_easy_setopt(curl,CURLOPT_CONNECTTIMEOUT,8L);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,1L);
    curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,2L);
    const char *ca=getenv("AURABAR_CA_BUNDLE");
    if (ca && *ca) curl_easy_setopt(curl,CURLOPT_CAINFO,ca);
    curl_easy_setopt(curl,CURLOPT_WRITEFUNCTION,stream_write);
    curl_easy_setopt(curl,CURLOPT_WRITEDATA,s);
    curl_easy_setopt(curl,CURLOPT_NOPROGRESS,0L);
    curl_easy_setopt(curl,CURLOPT_XFERINFOFUNCTION,stream_progress);
    curl_easy_setopt(curl,CURLOPT_XFERINFODATA,s);
    CURLcode code=curl_easy_perform(curl);
    int ret=s->failed;
    if (!ret && code==CURLE_OK) { ret=decode_stream(s,1)?s->failed:0; if(!s->decoded)ret=-2; }
    else if (!ret) ret=-2;
    curl_easy_cleanup(curl); free(s); return ret;
}
static int play_noise(const char *source,unsigned generation,pcm_output *pcm) {
    uint32_t rng=0x41555241U;
    float brown=0;
    double phase=0;
    int16_t samples[1024];
    for (;;) {
        for (unsigned i=0;i<1024;++i) {
            rng^=rng<<13; rng^=rng>>17; rng^=rng<<5;
            float white=((int32_t)rng)/2147483648.0f;
            brown=0.985f*brown+0.015f*white;
            double value=brown*18000;
            if (!strcmp(source,"noise:rain")) value=white*1800+brown*10000;
            if (!strcmp(source,"noise:ocean")) value=brown*(10000+8000*sin(phase));
            samples[i]=(int16_t)value;
            phase+=2*3.141592653589793/(22050*7);
        }
        int ret=output(pcm,samples,1024,22050,1,generation);
        if (ret) return ret;
    }
}
static void *play_worker(void *unused) {
    (void)unused;
    pcm_output pcm={0};
    for (;;) {
        pthread_mutex_lock(&player.lock);
        while (!player.quit && !player.wanted) pthread_cond_wait(&player.changed,&player.lock);
        if (player.quit) { pthread_mutex_unlock(&player.lock); break; }
        unsigned generation=player.generation;
        track current=player.tracks[player.view.index];
        player.view.position=player.view.duration=0;
        player.view.error[0]=0;
        player.view.state=MUSIC_BUFFERING;
        pthread_mutex_unlock(&player.lock);
        int ret;
        if (!strncmp(current.source,"noise:",6)) ret=play_noise(current.source,generation,&pcm);
        else if (!strncmp(current.source,"http://",7)||!strncmp(current.source,"https://",8)) ret=play_stream(current.source,generation,&pcm);
        else {
            const char *ext=strrchr(current.source,'.');
            ret=ext && !strcasecmp(ext,".wav")?play_wav(current.source,generation,&pcm):play_mp3(current.source,generation,&pcm);
        }
        close_output(&pcm);
        pthread_mutex_lock(&player.lock);
        if (generation==player.generation) {
            if (!ret && (player.view.index+1<player.view.count || player.view.repeat)) {
                player.view.index=(player.view.index+1)%player.view.count;
                snprintf(player.view.title,sizeof(player.view.title),"%s",player.tracks[player.view.index].title);
                ++player.generation;
            } else {
                player.wanted=0;
                player.view.state=ret==-2?MUSIC_ERROR:MUSIC_STOPPED;
                if (ret==-2) snprintf(player.view.error,sizeof(player.view.error),"Cannot play. Check file, network, CA or audio device.");
            }
        }
        pthread_mutex_unlock(&player.lock);
    }
    close_output(&pcm); return NULL;
}
static int compare_tracks(const void *a,const void *b) { return strcasecmp(((const track*)a)->title,((const track*)b)->title); }
static void add_track(const char *source,const char *title) {
    if (player.view.count>=MUSIC_MAX_TRACKS || strlen(source)>=sizeof(player.tracks[0].source)) return;
    track *t=&player.tracks[player.view.count++];
    snprintf(t->source,sizeof(t->source),"%s",source);
    snprintf(t->title,sizeof(t->title),"%s",title);
}
int music_reload(void) {
    if (!player.initialized) return -1;
    pthread_mutex_lock(&player.lock);
    if (player.wanted || player.output_active) { pthread_mutex_unlock(&player.lock); return -1; }
    player.view.count=0;
    const char *dir=getenv("AURABAR_MUSIC_DIR");
    if (!dir || !*dir) dir=PROJECT_MUSIC_URL;
    DIR *d=opendir(dir);
    if (d) {
        struct dirent *entry;
        while ((entry=readdir(d))) {
            const char *ext=strrchr(entry->d_name,'.');
            if (!ext || (strcasecmp(ext,".mp3") && strcasecmp(ext,".wav"))) continue;
            char path[512]; struct stat st;
            if (snprintf(path,sizeof(path),"%s/%s",dir,entry->d_name)>=(int)sizeof(path)) continue;
            if (!stat(path,&st) && S_ISREG(st.st_mode)) add_track(path,entry->d_name);
        }
        closedir(d);
    }
    qsort(player.tracks,(size_t)player.view.count,sizeof(track),compare_tracks);
    char path[512];
    if (snprintf(path,sizeof(path),"%s/stations.m3u",dir)<(int)sizeof(path)) {
        FILE *f=fopen(path,"r");
        if (f) {
            char line[768], title[128]="";
            while (fgets(line,sizeof(line),f)) {
                line[strcspn(line,"\r\n")]=0;
                if (!strncmp(line,"#EXTINF:",8)) {
                    char *comma=strchr(line,',');
                    snprintf(title,sizeof(title),"%s",comma?comma+1:"Radio");
                } else if (!strncmp(line,"http://",7)||!strncmp(line,"https://",8)) {
                    add_track(line,*title?title:"Network MP3 stream"); title[0]=0;
                }
            }
            fclose(f);
        }
    }
    add_track("noise:brown","Brown noise");
    add_track("noise:rain","Rain texture");
    add_track("noise:ocean","Ocean texture");
    player.view.index=0; player.view.state=MUSIC_STOPPED;
    player.view.position=player.view.duration=0;
    snprintf(player.view.title,sizeof(player.view.title),"%s",player.tracks[0].title);
    pthread_mutex_unlock(&player.lock); return 0;
}
int music_init(void) {
    if (player.initialized) return 0;
    memset(&player,0,sizeof(player)); player.seek_to=-1;
    pthread_mutex_init(&player.lock,NULL);
    pthread_condattr_t attr; pthread_condattr_init(&attr); pthread_condattr_setclock(&attr,CLOCK_MONOTONIC);
    pthread_cond_init(&player.changed,&attr); pthread_condattr_destroy(&attr);
    if (curl_global_init(CURL_GLOBAL_DEFAULT)) return -1;
    memset(&saved,0,sizeof(saved));
    if(file_param_read("music-favorites-v1.cfg",&saved,sizeof(saved)) ||
       saved.magic!=0x41554631U || saved.count>MUSIC_MAX_TRACKS) {
        memset(&saved,0,sizeof(saved)); saved.magic=0x41554631U;
    }
    for(unsigned i=0;i<saved.count;++i)saved.source[i][511]=0;
    player.initialized=1;
    music_reload();
    if (pthread_create(&player.worker,NULL,play_worker,NULL)) {
        player.initialized=0; pthread_cond_destroy(&player.changed); pthread_mutex_destroy(&player.lock); return -1;
    }
    return 0;
}
void music_shutdown(void) {
    if (!player.initialized) return;
    pthread_mutex_lock(&player.lock); player.quit=1; ++player.generation;
    pthread_cond_broadcast(&player.changed); pthread_mutex_unlock(&player.lock);
    pthread_join(player.worker,NULL);
    pthread_cond_destroy(&player.changed); pthread_mutex_destroy(&player.lock);
    player.initialized=0;
}
void music_get_snapshot(music_snapshot *out) {
    if (!player.initialized) { memset(out,0,sizeof(*out)); return; }
    pthread_mutex_lock(&player.lock);
    *out=player.view;
    out->favorite=player.view.count && is_favorite(player.tracks[player.view.index].source);
    out->sleep_seconds=player.sleep_at>now()?(unsigned)(player.sleep_at-now()):0;
    pthread_mutex_unlock(&player.lock);
}
int music_track_title(int index,char *dst,unsigned size) {
    if (!player.initialized || !dst || !size) return -1;
    pthread_mutex_lock(&player.lock);
    int ret=-1;
    if (index>=0 && index<player.view.count) { snprintf(dst,size,"%s",player.tracks[index].title); ret=0; }
    pthread_mutex_unlock(&player.lock); return ret;
}
int music_play(int index) {
    if (!player.initialized) return -1;
    pthread_mutex_lock(&player.lock);
    int ret=-1;
    if (index>=0 && index<player.view.count) {
        player.view.index=index; player.wanted=1; player.paused=0; player.seek_to=-1;
        player.view.position=player.view.duration=0; player.view.state=MUSIC_BUFFERING; player.view.error[0]=0;
        snprintf(player.view.title,sizeof(player.view.title),"%s",player.tracks[index].title);
        ++player.generation; pthread_cond_broadcast(&player.changed); ret=0;
    }
    pthread_mutex_unlock(&player.lock); return ret;
}
void music_stop(void) {
    if (!player.initialized) return;
    pthread_mutex_lock(&player.lock);
    player.wanted=player.paused=0; player.seek_to=-1; ++player.generation;
    player.view.state=MUSIC_STOPPED; player.view.position=0; player.view.error[0]=0;
    pthread_cond_broadcast(&player.changed); pthread_mutex_unlock(&player.lock);
}
void music_toggle_pause(void) {
    if (!player.initialized) return;
    pthread_mutex_lock(&player.lock);
    int index=player.view.index, start=!player.wanted;
    if (!start) { player.paused=!player.paused; pthread_cond_broadcast(&player.changed); }
    pthread_mutex_unlock(&player.lock);
    if (start) music_play(index);
}
void music_next(int delta) {
    music_snapshot s; music_get_snapshot(&s);
    if (s.count) music_play((s.index+delta%s.count+s.count)%s.count);
}
void music_seek(double seconds) {
    if (!player.initialized || !isfinite(seconds)) return;
    pthread_mutex_lock(&player.lock);
    if (player.wanted && player.view.duration>0) {
        if (seconds<0) seconds=0;
        if (seconds>player.view.duration) seconds=player.view.duration;
        player.seek_to=seconds;
    }
    pthread_mutex_unlock(&player.lock);
}
void music_set_repeat(bool repeat) {
    if (!player.initialized) return;
    pthread_mutex_lock(&player.lock); player.view.repeat=repeat; pthread_mutex_unlock(&player.lock);
}
void music_toggle_favorite(void) {
    if(!player.initialized)return;
    pthread_mutex_lock(&player.lock);
    const char *source=player.tracks[player.view.index].source;
    unsigned i=0;
    while(i<saved.count && strcmp(saved.source[i],source))++i;
    if(i<saved.count) {
        for(unsigned n=i+1;n<saved.count;++n)memcpy(saved.source[n-1],saved.source[n],512);
        --saved.count;
    } else if(saved.count<MUSIC_MAX_TRACKS) {
        snprintf(saved.source[saved.count++],512,"%s",source);
    }
    favorites snapshot=saved;
    pthread_mutex_unlock(&player.lock);
    if(file_param_write("music-favorites-v1.cfg",&snapshot,sizeof(snapshot)))
        fprintf(stderr,"Could not save music favorites\n");
}
void music_sleep_after(unsigned minutes) {
    if (!player.initialized) return;
    if (minutes>1440) minutes=1440;
    pthread_mutex_lock(&player.lock); player.sleep_at=minutes?now()+60*minutes:0;
    pthread_cond_broadcast(&player.changed); pthread_mutex_unlock(&player.lock);
}
int music_notification_begin(void) {
    if (!player.initialized) return 0;
    pthread_mutex_lock(&player.lock); player.notification=1;
    pthread_cond_broadcast(&player.changed);
    struct timespec until; clock_gettime(CLOCK_MONOTONIC,&until); until.tv_sec+=3;
    while (player.output_active) {
        if (pthread_cond_timedwait(&player.changed,&player.lock,&until)==ETIMEDOUT) {
            player.notification=0; pthread_cond_broadcast(&player.changed);
            pthread_mutex_unlock(&player.lock); return -1;
        }
    }
    pthread_mutex_unlock(&player.lock); return 0;
}
void music_notification_end(void) {
    if (!player.initialized) return;
    pthread_mutex_lock(&player.lock); player.notification=0;
    pthread_cond_broadcast(&player.changed); pthread_mutex_unlock(&player.lock);
}
