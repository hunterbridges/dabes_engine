#ifndef __engine_h
#define __engine_h

#include <sys/time.h>
#include <lcthw/list.h>
#include "../prefix.h"
#include "console.h"
#include "../audio/audio.h"
#include "../scripting/scripting.h"
#include "../input/input.h"
#include "../graphics/graphics.h"
#include "../physics/physics.h"
#include "easer.h"

typedef struct EngineTimer {
    struct timeval started_at;
    struct timeval paused_at;
    unsigned long pause_skip;
    int paused;
} EngineTimer;

typedef char *(*Engine_resource_path_func)(const char *filename);
typedef struct Engine {
    Audio *audio;
    Console *console;
    Input *input;
    Graphics *graphics;
    Physics *physics;
    Scripting *scripting;

    List *easers;

    Engine_resource_path_func resource_path;
    Engine_resource_path_func project_path;
    EngineTimer timer;

    short int reg_initialized;
    short int frame_now;
    long unsigned int frame_ticks;
    long unsigned int frame_skip;
    long unsigned int last_frame_at;
} Engine;

Engine *Engine_create(Engine_resource_path_func path_func,
                      Engine_resource_path_func project_path_func,
                      ConsoleProto console_proto,
                      const char *boot_script, void **sdl_screen);
void Engine_set_resource_path(Engine *engine,
                              Engine_resource_path_func resource_path);
void Engine_set_project_path(Engine *engine,
                             Engine_resource_path_func project_path);
void Engine_destroy(Engine *engine);
int Engine_bootstrap(Engine **engine, void **sdl_screen);
void Engine_regulate(Engine *engine);

void Engine_pause_time(Engine *engine);
void Engine_resume_time(Engine *engine);
unsigned long Engine_get_ticks(Engine *engine);

struct Scene;
struct Scene *Engine_get_current_scene(Engine *engine);
void Engine_frame_end(Engine *engine);

Easer *Engine_gen_easer(Engine *engine, int length_ms, Easer_curve curve);
void Engine_update_easers(Engine *engine);

FILE *Engine_open_resource(Engine *engine, char *filename);
int Engine_load_resource(Engine *engine, char *filename, unsigned char **out,
                         GLint *size);

#ifdef DABES_IOS
#define Engine_log(A, ...) Engine_log_iOS(A, ##__VA_ARGS__)
void Engine_log_iOS(char *fmt, ...);
#else
#define Engine_log(A, ...) debug(A, ##__VA_ARGS__)
#endif

#endif
