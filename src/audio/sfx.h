#ifndef __sfx_h
#define __sfx_h
#include <OpenAL/al.h>
#include <pthread.h>
#include "ogg_stream.h"

typedef struct Sfx {
    double volume;
    double vol_adjust;
    int playing;
    int _needs_play;

    int ended;
    int _needs_end;

    int initialized;
    ALuint source;

    pthread_mutex_t lock;

    char *filename;
    OggStream *ogg_stream;
} Sfx;

struct Audio;

#pragma mark - Main Thread
Sfx *Sfx_load(char *filename);
void Sfx_destroy(Sfx *sfx);
void Sfx_play(Sfx *sfx);
void Sfx_end(Sfx *sfx);
void Sfx_set_volume(Sfx *sfx, double volume);
void Sfx_set_vol_adjust(Sfx *sfx, double vol_adjust);

#pragma mark - Audio thread
void Sfx_t_update(Sfx *sfx);

#endif
