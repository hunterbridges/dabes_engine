#ifndef __engine_h
#define __engine_h

#include "../prefix.h"
#include "audio.h"
#include "../input/input.h"
#include "../graphics/graphics.h"
#include "../physics/physics.h"

typedef struct Engine {
    Object proto;
    Audio *audio;
    Input *input;
    Graphics *graphics;
    Physics *physics;

    short int reg_initialized;
    short int frame_now;
    long unsigned int frame_ticks;
    long unsigned int frame_skip;
    long unsigned int last_frame_at;

} Engine;

int Engine_init(void *self);
void Engine_destroy(void *self);
int Engine_bootstrap(Engine **engine, SDL_Surface **screen);
void Engine_regulate(Engine *engine);

extern Object EngineProto;

#endif
