#ifndef __music_h
#define __music_h
#include <lcthw/list.h>
#include <OpenAL/al.h>
#include <pthread.h>
#include "ogg_stream.h"

struct Scene;
typedef struct Music {
    double volume;
    List *ogg_streams;
    int playing;
    int _needs_play;

    int ended;
    int _needs_end;

    int loop;
    int initialized;
    ALuint source;
    OggStream *active_stream;

    struct Scene *scene;

    pthread_mutex_t lock;

    int num_files;
    char *ogg_files[];
} Music;

#pragma mark - Main thread
Music *Music_load(int num_files, char *ogg_files[]);
void Music_destroy(Music *music);
void Music_play(Music *music);
void Music_pause(Music *music);
void Music_end(Music *music);
void Music_set_volume(Music *music, double volume);
void Music_set_loop(Music *music, int loop);

#pragma mark - Audio thread
void Music_t_update(Music *music);

#endif
