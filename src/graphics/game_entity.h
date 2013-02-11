#ifndef __game_entity_h
#define __game_entity_h
#include "../prefix.h"
#include "../physics/fixture.h"
#include "../input/controller.h"

typedef struct GameEntity {
    Object proto;
    Fixture *fixture;
    Controller *controller;
    Uint32 color;
    SDL_Rect (*rect)(void *self);
    GLfloat alpha;
    GLuint texture;
} GameEntity;

int GameEntity_init(void *self);
void GameEntity_destroy(void *self);
void GameEntity_render(GameEntity *self, void *engine);
void GameEntity_control(GameEntity *entity, Input *input);

extern Object GameEntityProto;

#endif
