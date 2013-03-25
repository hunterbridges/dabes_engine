#ifndef __scene_h
#define __scene_h
#include "../prefix.h"
#include "game_entity.h"
#include "camera.h"
#include "../core/audio.h"
#include "../physics/world.h"

typedef struct Scene {
    Object proto;
    GLuint bg_texture;

    Music *music;
    Camera *camera;

    short int draw_grid;

    World *world;
    List *entities;
} Scene;

int Scene_init(void *self);
void Scene_destroy(void *self);
void Scene_update(Scene *scene, void *engine);
void Scene_render(Scene *scene, void *engine);
void Scene_control(Scene *scene, Input *input);
World *Scene_create_world(Scene *scene, Physics *physics);
void Scene_reset_camera(Scene *scene);
  
extern Object SceneProto;

#endif
