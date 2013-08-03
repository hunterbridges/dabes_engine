#ifndef __audio_h
#define __audio_h
#include <pthread.h>
#include <lcthw/list.h>
#if defined(DABES_IOS) || defined(DABES_MAC)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <al/al.h>
#include <al/alc.h>
#endif

#include "../prefix.h"
#include "ogg_stream.h"

typedef struct Audio {
    List *musics;
    List *active_sfx;
    ALCdevice *device;
    ALCcontext *context;

    pthread_t thread;
    pthread_mutex_t run_lock;
    pthread_mutex_t music_lock;
    pthread_mutex_t sfx_lock;
} Audio;

Audio *Audio_create();
int Audio_check();

struct Engine;
void Audio_destroy(Audio *audio);

struct Music;
struct Music *Audio_gen_music(Audio *audio, int num_files, char *files[]);
void Audio_destroy_music(Audio *audio, struct Music *music);

struct Sfx;
struct Sfx *Audio_gen_sfx(Audio *audio, char *filename);
void Audio_destroy_sfx(Audio *audio, struct Sfx *sfx);

void Audio_sweep(Audio *audio, struct Engine *engine);

#endif
