#ifndef __game_entity_h
#define __game_entity_h
#include "../prefix.h"

typedef struct GameEntity {
    Object proto;
    float x;
    float y;
    int width;
    int height;
    float xvelo;
    float yvelo;
    int mass;
    float time_scale;
    Uint32 color;
    SDL_Rect (*rect)(void *self);
} GameEntity;

int GameEntity_init(void *self);
void GameEntity_calc_physics(void *self, int ticks);
void GameEntity_render(void *self);

extern Object GameEntityProto;

#endif
