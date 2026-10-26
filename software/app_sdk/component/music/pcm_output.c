#define _POSIX_C_SOURCE 200809L
#include "pcm_output.h"
#include "em_hal_audio.h"
#include <dlfcn.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef int (*open_fn)(void **, const char *, int, int);
typedef int (*params_fn)(void *, int, int, unsigned, unsigned, int, unsigned);
typedef long (*write_fn)(void *, const void *, unsigned long);
typedef int (*recover_fn)(void *, int, int);
typedef int (*close_fn)(void *);
static void *library;
static open_fn alsa_open;
static params_fn alsa_params;
static write_fn alsa_write;
static recover_fn alsa_recover;
static close_fn alsa_drop, alsa_close;
static int (*format_value)(const char *);
static int load(void) {
    if (library) return 0;
    library=dlopen("libasound.so.2", RTLD_NOW | RTLD_LOCAL);
    if (!library) return -1;
#define LOAD(dst, name) do { *(void **)(&(dst))=dlsym(library,name); if (!(dst)) goto fail; } while(0)
    LOAD(alsa_open,"snd_pcm_open");
    LOAD(alsa_params,"snd_pcm_set_params");
    LOAD(alsa_write,"snd_pcm_writei");
    LOAD(alsa_recover,"snd_pcm_recover");
    LOAD(alsa_drop,"snd_pcm_drop");
    LOAD(alsa_close,"snd_pcm_close");
    LOAD(format_value,"snd_pcm_format_value");
#undef LOAD
    return 0;
fail:
    dlclose(library); library=NULL; return -1;
}
int pcm_output_open(pcm_output *out, unsigned rate, unsigned channels) {
    if (out->pcm && out->rate==rate && out->channels==channels) return 0;
    pcm_output_close(out);
    if (rate<8000 || rate>192000 || (channels!=1 && channels!=2) || load()) return -1;
    const char *device=getenv("AURABAR_AUDIO_DEVICE");
    if (!device || !*device) device="default";
    if (alsa_open(&out->pcm,device,0,0)<0) { out->pcm=NULL; return -1; }
    /* Stable ALSA access enum: SND_PCM_ACCESS_RW_INTERLEAVED=3. */
    if (alsa_params(out->pcm,format_value("S16_LE"),3,channels,rate,1,100000)<0) {
        alsa_close(out->pcm); out->pcm=NULL; return -1;
    }
    out->rate=rate; out->channels=channels;
    em_hal_audio_output_enable(1);
    return 0;
}
int pcm_output_write(pcm_output *out, const int16_t *samples, size_t frames) {
    size_t offset=0;
    while (offset<frames) {
        long n=alsa_write(out->pcm,samples+offset*out->channels,frames-offset);
        if (n<0) { if (alsa_recover(out->pcm,(int)n,1)<0) return -1; continue; }
        if (!n) return -1;
        offset+=(size_t)n;
    }
    /* ALSA null is used by automated tests and must preserve playback pacing. */
    const char *device=getenv("AURABAR_AUDIO_DEVICE");
    if (device && !strcmp(device,"null")) {
        long ns=(long)(1000000000.0*frames/out->rate);
        struct timespec t={ns/1000000000L,ns%1000000000L};
        nanosleep(&t,NULL);
    }
    return 0;
}
void pcm_output_close(pcm_output *out) {
    if (!out->pcm) return;
    alsa_drop(out->pcm); alsa_close(out->pcm);
    out->pcm=NULL; out->rate=out->channels=0;
    em_hal_audio_output_enable(0);
}
