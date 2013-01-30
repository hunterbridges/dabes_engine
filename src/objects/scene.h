#ifndef __scene_h
#define __scene_h
#include "../prefix.h"
#include "game_entity.h"

typedef struct Scene {
    Object proto;
    GameEntity *things[256];
} Scene;

int Scene_init(void *self);
void Scene_calc_physics(void *self, int ticks);
void Scene_destroy(void *self);

extern Object SceneProto;

#endif
