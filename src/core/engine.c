#include "engine.h"
#include "../scenes/scene.h"

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

    gettimeofday(&(engine->timer.started_at), NULL);
    engine->timer.pause_skip = 0;
    engine->timer.paused = 0;

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

int Engine_bootstrap(Engine **engine, void **sdl_screen) {
#ifdef DABES_SDL
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    TTF_Init();
    *sdl_screen =
        SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_OPENGL);

    check(Graphics_init_GL(SCREEN_WIDTH, SCREEN_HEIGHT) == 1, "Init OpenGL");
#endif

    *engine = NEW(Engine, "The game engine");
    return 1;
error:
    return 0;
}

uint32_t tick_diff(struct timeval earlier, struct timeval later) {
  uint32_t ticks;
  ticks =
      (later.tv_sec - earlier.tv_sec) * 1000 +
      (later.tv_usec - earlier.tv_usec) / 1000;
  return ticks;
}

void Engine_pause_time(Engine *engine) {
    if (engine->timer.paused) return;
    struct timeval now;

    gettimeofday(&now, NULL);
    engine->timer.paused_at = now;
    engine->timer.paused = 1;
}

void Engine_resume_time(Engine *engine) {
    if (!engine->timer.paused) return;
    struct timeval now;

    gettimeofday(&now, NULL);
    engine->timer.pause_skip += tick_diff(engine->timer.paused_at, now);
    engine->timer.paused = 0;
}

uint32_t Engine_get_ticks(Engine *engine) {
    uint32_t ticks;
    struct timeval now;

    gettimeofday(&now, NULL);
    ticks = tick_diff(engine->timer.started_at, now) -
        engine->timer.pause_skip;
    return ticks;
}

void Engine_regulate(Engine *engine) {
    check_mem(engine);

    long unsigned int ticks = Engine_get_ticks(engine);
    long unsigned int ticks_since_last = ticks - engine->last_frame_at;
    engine->frame_now = 0;

    if (engine->reg_initialized == 0) {
        engine->last_frame_at = ticks;
        engine->reg_initialized = 1;
        engine->frame_now = 1;
        engine->frame_ticks = ticks_since_last;
    } else {
        if (engine->timer.paused) return;
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
