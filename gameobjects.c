#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "dbg.h"
#include "gameobjects.h"

typedef struct PhysForce {
  double trajectory;
  int magnitude;
} PhysForce;

int Thing_init(void *self) {
    check_mem(self);
    debug("blah");
    Thing *thing = (Thing *)self;
    thing->x = 270;
    thing->y = 0;
    thing->mass = 100;
    return 1;

error:
    return 0;
}

void Thing_calc_physics(void *self, int ticks) {
    check_mem(self);
    Thing *thing = (Thing *)self;

// for each force
    int xaccel = 0;
    int yaccel = 0;
    PhysForce *gravity = calloc(1, sizeof(PhysForce));
    gravity->trajectory = M_PI / 2.0;
    gravity->magnitude = 10 * thing->mass;

    PhysForce *ground = calloc(1, sizeof(PhysForce));
    if (thing->y >= 380) {
        ground->trajectory = 3 * M_PI / 2.0;
        ground->magnitude = gravity->magnitude;
    }

    PhysForce *right = calloc(1, sizeof(PhysForce));
    right->trajectory = 0;
    right->magnitude = 3 * thing->mass;

    PhysForce *forces[] = { gravity, ground, right };
    int i = 0;
    for (i = 0; i < 3; i++) {
      PhysForce *force = forces[i];
      xaccel += cos(force->trajectory) * force->magnitude / thing->mass;
      yaccel += sin(force->trajectory) * force->magnitude / thing->mass;
    }

    thing->xvelo += xaccel * ticks / 100;
    thing->yvelo += yaccel * ticks / 100;

    // debug("Force xa = %d, ya = %d, xv = %d, yv = %d", xaccel, yaccel, thing->xvelo, thing->yvelo);

    thing->x += thing->xvelo * ticks / 100;
    thing->y += thing->yvelo * ticks / 100;

    free(gravity);
    free(ground);

    if (thing->y > 380) {
      thing->yvelo = 0;
      thing->y = 380;
    }
error:
    return;
}

void Game_calc_physics(void *self, int ticks) {
    check_mem(self);
    Game *game = (Game *)self;
    game->thing->_(calc_physics)(game->thing, ticks);

error:
    return;
}

void Game_destroy(void *self) {
    check_mem(self);
    Game *game = (Game *)self;
    game->thing->_(destroy)(game->thing);

    Object_destroy(self);
error:
    return;
}
