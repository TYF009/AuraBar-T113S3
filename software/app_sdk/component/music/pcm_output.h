#ifndef AURABAR_PCM_OUTPUT_H
#define AURABAR_PCM_OUTPUT_H
#include <stddef.h>
#include <stdint.h>
typedef struct { void *pcm; unsigned rate, channels; } pcm_output;
int pcm_output_open(pcm_output *, unsigned rate, unsigned channels);
int pcm_output_write(pcm_output *, const int16_t *samples, size_t frames);
void pcm_output_close(pcm_output *);
#endif
