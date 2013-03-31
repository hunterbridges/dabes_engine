#ifndef __game_entity_h
#define __game_entity_h
#include "../prefix.h"
#include "../physics/fixture.h"
#include "../input/input.h"
#include "../input/controller.h"

typedef struct GameEntity {
    Object proto;
    Fixture *fixture;
    Controller *controller;
    uint32_t color;
    GLfloat alpha;
    GfxTexture *texture;
} GameEntity;

int GameEntity_init(void *self);
void GameEntity_destroy(void *self);
void GameEntity_render(GameEntity *self, void *engine);
void GameEntity_assign_controller(GameEntity *entity, Controller *controller);
void GameEntity_control(GameEntity *entity, Input *input);
GfxPoint GameEntity_center(GameEntity *entity);

extern Object GameEntityProto;

#endif
