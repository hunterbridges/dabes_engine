#ifndef __music_h
#define __music_h
#include <lcthw/list.h>
#include "ogg_stream.h"
#include <OpenAL/al.h>

typedef struct Music {
    double volume;
    List *ogg_streams;
    int playing;
    int ended;
    int loop;
    ALuint source;
    OggStream *active_stream;

    int num_files;
    char *ogg_files[];
} Music;

Music *Music_load(int num_files, char *ogg_files[]);
void Music_destroy(Music *music);
void Music_play(Music *music);
void Music_update(Music *music);
void Music_pause(Music *music);
void Music_end(Music *music);
void Music_set_volume(Music *music, double volume);
void Music_set_loop(Music *music, int loop);

#endif
