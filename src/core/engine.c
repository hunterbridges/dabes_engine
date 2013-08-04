#include "engine.h"
#include "../scenes/scene.h"
#include "../scenes/scene_bindings.h"

char *Engine_default_resource_path(const char *filename) {
  const char *resources = "resource/";
  char *newpath = calloc(strlen(resources) + strlen(filename) + 1, sizeof(char));
  strcpy(newpath, resources);
  strcat(newpath, filename);
  return newpath;
}

char *Engine_default_project_path(const char *filename) {
  const char *project = "project/";
  char *newpath = calloc(strlen(project) + strlen(filename) + 1, sizeof(char));
  strcpy(newpath, project);
  strcat(newpath, filename);
  return newpath;
}

Engine *Engine_create(Engine_resource_path_func path_func,
                      Engine_resource_path_func project_path_func,
                      const char *boot_script, void **sdl_screen) {
    Engine *engine = calloc(1, sizeof(Engine));
    check(engine != NULL, "Could not create engine. World explodes.");

#ifdef DABES_SDL
    if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) == -1) {
        log_err("%s", SDL_GetError());
        exit(1);
    }
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    *sdl_screen =
        SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 32, SDL_OPENGL);
#endif
    check(Graphics_init_GL(SCREEN_WIDTH, SCREEN_HEIGHT) == 1, "Init OpenGL");

    engine->resource_path =
        path_func ? path_func : Engine_default_resource_path;
    engine->project_path =
        project_path_func ? project_path_func : Engine_default_project_path;

    engine->audio = Audio_create();
    engine->input = Input_create();
    engine->graphics = Graphics_create(engine);
    engine->physics = Physics_create();
    engine->easers = List_create();

    engine->scripting = Scripting_create(engine, boot_script);
    check(engine == luaL_get_engine(engine->scripting->L),
            "Did not properly register engine in scripting subsystem");

    engine->reg_initialized = 0;
    engine->frame_now = 0;
    engine->frame_skip = 1000 / FPS;
    engine->last_frame_at = 0;
    engine->frame_ticks = 0;

    gettimeofday(&(engine->timer.started_at), NULL);
    engine->timer.pause_skip = 0;
    engine->timer.paused = 0;

    return engine;
error:
    if (engine) Engine_destroy(engine);
    return NULL;
}

void Engine_destroy(Engine *engine) {
    check(engine != NULL, "No engine to destroy");

    List_clear_destroy(engine->easers);

    // Scripting has to go first, as it
    // manages all the objects that leverage other things.
    Scripting_destroy(engine->scripting);

    Audio_destroy(engine->audio);
    Input_destroy(engine->input);
    Graphics_destroy(engine->graphics);
    Physics_destroy(engine->physics);

#ifdef DABES_SDL
    SDL_Quit();
#endif

    free(engine);
    return;
error:
    free(engine);
}

void Engine_set_resource_path(Engine *engine,
                              Engine_resource_path_func resource_path) {
    engine->resource_path = resource_path;
    Scripting_update_paths(engine->scripting, engine);
}

void Engine_set_project_path(Engine *engine,
                             Engine_resource_path_func project_path) {
    engine->project_path = project_path;
    Scripting_update_paths(engine->scripting, engine);
}

unsigned long tick_diff(struct timeval earlier, struct timeval later) {
  unsigned long ticks;
  ticks =
      (later.tv_sec - earlier.tv_sec) * 1000 +
      (later.tv_usec - earlier.tv_usec) / 1000;
  return ticks;
}

void Engine_pause_time(Engine *engine) {
    if (!engine) return;
    if (engine->timer.paused) return;
    struct timeval now;

    gettimeofday(&now, NULL);
    engine->timer.paused_at = now;
    engine->timer.paused = 1;
}

void Engine_resume_time(Engine *engine) {
    if (!engine) return;
    if (!engine->timer.paused) return;
    struct timeval now;

    gettimeofday(&now, NULL);
    engine->timer.pause_skip += tick_diff(engine->timer.paused_at, now);
    engine->timer.paused = 0;
}

unsigned long Engine_get_ticks(Engine *engine) {
    unsigned long ticks;
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

Scene *Engine_get_current_scene(Engine *engine) {
    return luaL_get_current_scene(engine->scripting->L);
}

void Engine_frame_end(Engine *engine) {
    luaL_flip_scene(engine->scripting->L);
}

Easer *Engine_gen_easer(Engine *engine, int length_ms, Easer_curve curve) {
    Easer *easer = Easer_create(length_ms, curve);
    List_push(engine->easers, easer);
    return easer;
}

void Engine_update_easers(Engine *engine) {
    ListNode *node = engine->easers->first;
    while (node != NULL) {
        Easer *easer = node->value;

        if (easer->finished) {
            ListNode *old = node;
            node = node->next;
            List_remove(engine->easers, old);
            Easer_destroy(easer);
            continue;
        }

        Easer_update(easer, engine, engine->frame_ticks);
        node = node->next;
    }
}

FILE *Engine_open_resource(Engine *engine, char *filename) {
    char *rpath = engine->resource_path(filename);
    FILE *file = fopen(rpath, "r");
    free(rpath);
    return file;
}

int Engine_load_resource(Engine *engine, char *filename, unsigned char **out,
                         GLint *size) {
    unsigned char *output = NULL;
    FILE *file = Engine_open_resource(engine, filename);
    check(file != NULL, "Failed to open %s", filename);

    fseek(file, 0, SEEK_END);
    unsigned int sz = (unsigned int)ftell(file);
    rewind(file);

    output = malloc(sz * sizeof(unsigned char));
    check_mem(output);

    fread(output, 1, sz, file);
    fclose(file);
    output[sz] = '\0';

    *out = output;
    *size = sz;

    return 1;
error:
    if (file != NULL) fclose(file);
    if (output != NULL) free(output);
    return 0;
}
