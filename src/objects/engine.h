#ifndef __engine_h
#define __engine_h

#include "../prefix.h"
#include "audio.h"
#include "input.h"
#include "graphics.h"
#include "physics.h"

typedef struct Engine {
    Object proto;
    Audio *audio;
    Input *input;
    Graphics *graphics;
    Physics *physics;
} Engine;

int Engine_init(void *self);
void Engine_destroy(void *self);

extern Object EngineProto;

#endif
