#ifndef __scene_h
#define __scene_h
#include "../prefix.h"
#include "game_entity.h"
#include "../physics/world.h"

typedef struct Scene {
    Object proto;
    GLuint bg_texture;

#ifndef DABES_IOS
    Mix_Music *music;
#endif

    double projection_scale;
    double projection_rotation;

    short int draw_grid;

    World *world;
    List *entities;
} Scene;

int Scene_init(void *self);
void Scene_destroy(void *self);
void Scene_render(void *self, void *engine);
void Scene_control(Scene *scene, Input *input);
World *Scene_create_world(Scene *scene, Physics *physics);

extern Object SceneProto;

#endif
