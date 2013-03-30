#include "engine.h"
#include "../graphics/scene.h"

int Engine_init(void *self) {
    check_mem(self);
    Engine *engine = self;

    engine->audio = NEW(Audio, "Audio Engine");
    engine->input = NEW(Input, "Input Engine");
    engine->graphics = NEW(Graphics, "Graphics Engine");
    engine->physics = NEW(Physics, "Physics Engine");

    engine->reg_initialized = 0;
    engine->frame_now = 0;
    engine->frame_skip = 1000 / FPS;
    engine->last_frame_at = 0;
    engine->frame_ticks = 0;

    return 1;
error:
    return 0;
}

void Engine_destroy(void *self) {
    check_mem(self);
    Engine *engine = self;

    engine->audio->_(destroy)(engine->audio);
    engine->input->_(destroy)(engine->input);
    engine->graphics->_(destroy)(engine->graphics);
    engine->physics->_(destroy)(engine->physics);

    free(self);
    return;
error:
    free(self);
}

int Engine_bootstrap(Engine **engine, SDL_Surface **screen) {
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);


#ifndef DABES_IOS
    TTF_Init();
    *screen =
        SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_OPENGL);

    check(Graphics_init_GL(SCREEN_WIDTH, SCREEN_HEIGHT) == 1, "Init OpenGL");
#endif

    *engine = NEW(Engine, "The game engine");
    return 1;
error:
    return 0;
}

void Engine_regulate(Engine *engine) {
    check_mem(engine);

    long unsigned int ticks = SDL_GetTicks();
    long unsigned int ticks_since_last = ticks - engine->last_frame_at;
    engine->frame_now = 0;

    if (engine->reg_initialized == 0) {
        engine->last_frame_at = ticks;
        engine->reg_initialized = 1;
        engine->frame_now = 1;
        engine->frame_ticks = ticks_since_last;
    } else {
        if (ticks_since_last >= engine->frame_skip) {
            engine->frame_now = 1;
            engine->frame_ticks = ticks_since_last;
            engine->last_frame_at = ticks;
        }
    }

    return;
error:
    return;
}

Object EngineProto = {
    .init = Engine_init,
    .destroy = Engine_destroy
};
