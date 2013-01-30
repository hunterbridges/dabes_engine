#include "game_entity.h"

SDL_Rect GameEntity_rect(void *self) {
    GameEntity *thing = (GameEntity *)self;
    SDL_Rect rect = {roundf(thing->x), roundf(thing->y),
                     thing->width, thing->height};
    return rect;
}

int GameEntity_init(void *self) {
    check_mem(self);
    GameEntity *thing = (GameEntity *)self;
    thing->x = 270;
    thing->y = 0;
    thing->width = 100;
    thing->height = 100;
    thing->mass = 100;
    thing->time_scale = 1.0;

    thing->rect = GameEntity_rect;
    return 1;

error:
    return 0;
}

void GameEntity_calc_physics(void *self, int ticks) {
    check_mem(self);
    GameEntity *thing = (GameEntity *)self;

    int xaccel = 0;
    int yaccel = 0;

    if (thing->y < 480 - thing->height) {
        yaccel = 5;
    }

    float dt = (ticks / 2) * thing->time_scale / 100.0;

    thing->xvelo += xaccel * dt;
    thing->yvelo += yaccel * dt;

    thing->x += thing->xvelo;
    thing->y += thing->yvelo;

    thing->xvelo += xaccel * dt;
    thing->yvelo += yaccel * dt;

    if (thing->y >= 480 - thing->height) {
      thing->yvelo = 0;
      thing->y = 480 - thing->height;
    }
error:
    return;
}

Object GameEntityProto = {
   .init = GameEntity_init,
   .calc_physics = GameEntity_calc_physics,
};
