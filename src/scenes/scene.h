#ifndef __scene_h
#define __scene_h
#include <chipmunk/chipmunk.h>
#include <lcthw/darray.h>
#include "../prefix.h"
#include "../audio/audio.h"
#include "../audio/music.h"
#include "../core/engine.h"
#include "../entities/entity.h"
#include "../graphics/camera.h"
#include "../graphics/parallax.h"
#include "../graphics/tile_map.h"
#include "../physics/world.h"
#include "../math/vmatrix.h"

struct Scene;
typedef struct SceneProto {
    void (*start)(struct Scene *scene, Engine *engine);
    void (*stop)(struct Scene *scene, Engine *engine);
    void (*cleanup)(struct Scene *scene, Engine *engine);
    void (*update)(struct Scene *scene, Engine *engine);
    void (*render)(struct Scene *scene, Engine *engine);
    void (*control)(struct Scene *scene, Engine *engine);
    void (*add_entity)(struct Scene *scene, Engine *engine, Entity *entity);
    Entity *(*hit_test)(struct Scene *scene, VPoint g_point);
} SceneProto;

typedef enum {
  kSceneRenderModeNormal = 0,
  kSceneRenderModePhysicsDebug = 1
} SceneRenderMode;

typedef enum {
  kSceneNotSelecting = 0,
  kSceneSelectingForCamera
} SceneEntitySelectionMode;

typedef struct Scene {
    SceneProto proto;
    void *context;
    char *name;

    GfxTexture *bg_texture; // deprecated

    DArray *entities;
    Music *music;
    Camera *camera;
    union {
      World *world;
      cpSpace *space;
    };
    Parallax *parallax;
    TileMap *tile_map;

    VVector4 cover_color;

    short int draw_grid;
    short int debug_camera;
    short int render_mode;
    int started;

    int pixels_per_meter;
  
    SceneEntitySelectionMode selection_mode;
    List *selected_entities;
} Scene;

Scene *Scene_create(Engine *engine, SceneProto proto);
void Scene_destroy(Scene *scene, Engine *engine);
void Scene_restart(Scene *scene, Engine *engine);
void Scene_load_tile_map(Scene *scene, Engine *engine, char *map_file,
                         int abs_path, float meters_per_tile);
void Scene_set_tile_map(Scene *scene, Engine *engine, TileMap *tile_map);
void Scene_draw_debug_grid(Scene *scene, Graphics *graphics);
void Scene_reset_camera(Scene *scene, Engine *engine);
void Scene_render(Scene *scene, Engine *engine);
void Scene_update(Scene *scene, Engine *engine);
void Scene_control(Scene *scene, Engine *engine);

void Scene_set_selection_mode(Scene *scene, SceneEntitySelectionMode mode);
int Scene_select_entities_at(Scene *scene, VPoint screen_point);

#endif
