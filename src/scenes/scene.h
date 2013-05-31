#ifndef __scene_h
#define __scene_h
#include <chipmunk/chipmunk.h>
#include "../prefix.h"
#include "../audio/audio.h"
#include "../audio/music.h"
#include "../core/engine.h"
#include "../entities/entity.h"
#include "../graphics/camera.h"
#include "../graphics/parallax.h"
#include "../graphics/tile_map.h"
#include "../physics/world.h"

struct Scene;
typedef struct SceneProto {
    void (*start)(struct Scene *scene, Engine *engine);
    void (*stop)(struct Scene *scene, Engine *engine);
    void (*cleanup)(struct Scene *scene, Engine *engine);
    void (*update)(struct Scene *scene, Engine *engine);
    void (*render)(struct Scene *scene, Engine *engine);
    void (*control)(struct Scene *scene, Engine *engine);
    void (*add_entity)(struct Scene *scene, Engine *engine, Entity *entity);
} SceneProto;

typedef struct Scene {
    SceneProto proto;
    char *name;

    GfxTexture *bg_texture; // deprecated

    Music *music;
    Camera *camera;
    union {
      World *world;
      cpSpace *space;
    };
    Parallax *parallax;
    TileMap *tile_map;
    List *entities;

    short int draw_grid;
    short int debug_camera;
    int started;

    int pixels_per_meter;
} Scene;

Scene *Scene_create(Engine *engine, SceneProto proto);
void Scene_destroy(Scene *scene, Engine *engine);
void Scene_restart(Scene *scene, Engine *engine);
void Scene_load_tile_map(Scene *scene, Engine *engine, char *map_file,
                         int abs_path);
void Scene_set_tile_map(Scene *scene, Engine *engine, TileMap *tile_map);
void Scene_reset_camera(Scene *scene);
void Scene_draw_debug_grid(Scene *scene, Graphics *graphics);

#endif
