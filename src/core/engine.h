#ifndef __engine_h
#define __engine_h

#include <sys/time.h>
#include "../prefix.h"
#include "../audio/audio.h"
#include "../scripting/scripting.h"
#include "../input/input.h"
#include "../graphics/graphics.h"
#include "../physics/physics.h"

typedef struct EngineTimer {
    struct timeval started_at;
    struct timeval paused_at;
    uint32_t pause_skip;
    int paused;
} EngineTimer;

typedef struct Engine {
    Object proto;
    Audio *audio;
    Input *input;
    Graphics *graphics;
    Physics *physics;
    Scripting *scripting;

    EngineTimer timer;

    short int reg_initialized;
    short int frame_now;
    long unsigned int frame_ticks;
    long unsigned int frame_skip;
    long unsigned int last_frame_at;
} Engine;

Engine *Engine_create(const char *boot_script, void **sdl_screen);
void Engine_destroy(Engine *engine);
int Engine_bootstrap(Engine **engine, void **sdl_screen);
void Engine_regulate(Engine *engine);

void Engine_pause_time(Engine *engine);
void Engine_resume_time(Engine *engine);
uint32_t Engine_get_ticks(Engine *engine);

struct Scene;
struct Scene *Engine_get_current_scene(Engine *engine);

#ifdef DABES_IOS
#define Engine_log(A, ...) Engine_log_iOS(A, ##__VA_ARGS__)
void Engine_log_iOS(char *fmt, ...);
#else
#define Engine_log(A, ...) debug(A, ##__VA_ARGS__)
#endif

extern Object EngineProto;

#endif
