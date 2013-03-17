#ifndef __scene_h
#define __scene_h
#include <lcthw/liblcthw.h>
#include "../prefix.h"
#include "game_entity.h"
#include "../physics/world.h"

typedef struct Scene {
    Object proto;
    GLuint bg_texture;

    Mix_Music *music;

    double projection_scale;
    double projection_rotation;

    List *entities;
} Scene;

int Scene_init(void *self);
void Scene_destroy(void *self);
void Scene_render(void *self, void *engine);
void Scene_control(Scene *scene, Input *input);
World *Scene_create_world(Scene *scene, Physics *physics);

extern Object SceneProto;

#endif
