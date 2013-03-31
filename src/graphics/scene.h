#ifndef __scene_h
#define __scene_h
#include "../prefix.h"
#include "../core/engine.h"
#include "game_entity.h"
#include "camera.h"
#include "tile_map.h"
#include "../core/audio.h"
#include "../physics/world.h"

typedef struct Scene {
    GfxTexture *bg_texture;

    Music *music;
    Camera *camera;

    short int draw_grid;

    World *world;
    TileMap *tile_map;
    List *entities;
  
    int started;
} Scene;

Scene *Scene_create(Engine *engine);
void Scene_destroy(Scene *scene);
void Scene_restart(Scene *scene, Engine *engine);
void Scene_update(Scene *scene, void *engine);
void Scene_render(Scene *scene, void *engine);
void Scene_control(Scene *scene, Input *input);
void Scene_load_tile_map(Scene *scene, Engine *engine, char *map_file,
                         int abs_path);
void Scene_set_tile_map(Scene *scene, Engine *engine, TileMap *tile_map);
World *Scene_create_world(Scene *scene, Physics *physics);
void Scene_reset_camera(Scene *scene);
  
#endif
