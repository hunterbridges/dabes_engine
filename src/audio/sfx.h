#ifndef __sfx_h
#define __sfx_h
#include "ogg_stream.h"
#include <OpenAL/al.h>

typedef struct Sfx {
    double volume;
    int playing;
    int ended;
    ALuint source;

    OggStream *ogg_stream;
} Sfx;

Sfx *Sfx_load(char *filename);
void Sfx_destroy(Sfx *sfx);
void Sfx_play(Sfx *sfx);
void Sfx_update(Sfx *sfx);
void Sfx_set_volume(Sfx *sfx, double volume);

#endif
