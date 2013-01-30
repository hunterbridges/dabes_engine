#ifndef __scene_h
#define __scene_h
#include "../prefix.h"
#include "game_entity.h"

typedef struct Scene {
    Object proto;
    GameEntity *things[256];
    GLuint bgTexture;

    Mix_Music *music;
} Scene;

int Scene_init(void *self);
void Scene_destroy(void *self);
void Scene_calc_physics(void *self, void *engine, int ticks);
void Scene_render(void *self, void *engine);

extern Object SceneProto;

#endif
