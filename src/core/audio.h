#ifndef __audio_h
#define __audio_h
#include "../prefix.h"

typedef struct Audio {
    Object proto;
} Audio;

int Audio_init(void *self);

extern Object AudioProto;

#endif
