#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "dbg.h"
#include "gameobjects.h"

typedef struct PhysForce {
  double trajectory;
  int magnitude;
} PhysForce;

SDL_Rect Thing_rect(void *self) {
    Thing *thing = (Thing *)self;
    SDL_Rect rect = {roundf(thing->x), roundf(thing->y),
                     thing->width, thing->height};
    return rect;
}

int Thing_init(void *self) {
    check_mem(self);
    Thing *thing = (Thing *)self;
    thing->x = 270;
    thing->y = 0;
    thing->width = 100;
    thing->height = 100;
    thing->mass = 100;
    thing->time_scale = 1.0;

    thing->rect = Thing_rect;
    return 1;

error:
    return 0;
}

void Thing_calc_physics(void *self, int ticks) {
    check_mem(self);
    Thing *thing = (Thing *)self;

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

void Game_calc_physics(void *self, int ticks) {
    check_mem(self);
    Game *game = (Game *)self;
    int i = 0;
    for (i = 0; i < 256; i++) {
        if (game->things[i] == NULL) break;
        Thing *thing = game->things[i];
        thing->_(calc_physics)(thing, ticks);
    }

error:
    return;
}

void Game_destroy(void *self) {
    check_mem(self);
    Game *game = (Game *)self;
    int i = 0;
    for (i = 0; i < 256; i++) {
        if (game->things[i] == NULL) break;
        Thing *thing = game->things[i];
        thing->_(destroy)(thing);
    }

    Object_destroy(self);
error:
    return;
}
