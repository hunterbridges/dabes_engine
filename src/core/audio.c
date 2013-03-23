#include "audio.h"

Music *Music_load(char *filename) {
  Music *music = malloc(sizeof(Music));
  music->volume = 1;

#ifdef DABES_IOS
    music->bridge = calloc(1, sizeof(struct AudioBridge));
    AudioBridge_load(music->bridge, filename);
#else
    music->mix_music = Mix_LoadMUS(filename);

    if (music->mix_music == NULL) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
    }
#endif
    Music_play(music);
    return music;
}

void Music_destroy(Music *music) {
#ifdef DABES_IOS
    AudioBridge_destroy(music->bridge);
#else
    Mix_FreeMusic(music->mix_music);
#endif
    free(music);
}

void Music_play(Music *music) {
#ifdef DABES_IOS
    AudioBridge_play(music->bridge);
#else
    Mix_PlayMusic(music->mix_music, -1);
#endif
}

void Music_pause(Music *music) {
#ifdef DABES_IOS
    AudioBridge_pause(music->bridge);
#else
    Mix_HaltMusic();
#endif
}

void Music_set_volume(Music *music, double volume) {
    music->volume = volume;
#ifdef DABES_IOS
    AudioBridge_set_volume(music->bridge, volume);
#else
    Mix_VolumeMusic(volume * 128.f);
#endif
}

int Audio_init(void *self) {
    check_mem(self);

#ifdef DABES_IOS
#else
   int audio_rate = 44100;
    Uint16 audio_format = AUDIO_S16;
    int audio_channels = 2;
    int audio_buffers = 4096;
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
        printf("Unable to open audio!\n");
        return 0;
    }
#endif

    return 1;
error:
    return 0;
}

Object AudioProto = {
    .init = Audio_init
};

