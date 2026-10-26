#ifndef _EM_HAL_AUDIO_H
#define _EM_HAL_AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

int em_get_audio_vol();
int em_set_audio_vol(int vol);
int em_play_audio(const char *url);
void em_stop_play_audio();
void em_hal_audio_output_enable(int enabled);

#ifdef __cplusplus
}
#endif

#endif
