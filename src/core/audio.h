#ifndef __audio_h
#define __audio_h
#include "../prefix.h"
#ifdef DABES_IOS
#include "AudioBridge.h"
#endif

typedef struct Music {
  double volume;
#ifdef DABES_IOS
  struct AudioBridge *bridge;
#else
  Mix_Music *mix_music;
#endif
} Music;

Music *Music_load(char *filename, char *loop);
void Music_destroy(Music *music);
void Music_play(Music *music);
void Music_pause(Music *music);
void Music_set_volume(Music *music, double volume);
  
typedef struct Audio {
    Object proto;
} Audio;

int Audio_init(void *self);

extern Object AudioProto;

#endif
