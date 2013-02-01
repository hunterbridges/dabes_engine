#ifndef __game_entity_h
#define __game_entity_h
#include "../prefix.h"
#include "fixture.h"

typedef struct GameEntity {
    Object proto;
    Fixture *fixture;
    Uint32 color;
    SDL_Rect (*rect)(void *self);
    GLfloat alpha;
} GameEntity;

int GameEntity_init(void *self);
void GameEntity_destroy(void *self);
void GameEntity_render(void *self, void *engine);

extern Object GameEntityProto;

#endif
