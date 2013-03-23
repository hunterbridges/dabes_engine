#include "audio.h"

int Audio_init(void *self) {
    check_mem(self);

#ifndef DABES_IOS
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

