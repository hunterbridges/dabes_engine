#ifndef __world_h
#define __world_h
#include "../prefix.h"
#include "physics.h"
#include "fixture.h"

typedef struct World {
    float height;
    float width;
    float time_scale;
    float pixels_per_meter;
    float gravity;
    float air_density;
    uint num_fixtures;
    Fixture *fixtures[256];
} World;

int World_init(void *self);
void World_destroy(void *self);
void World_solve(Physics *physics, World *world, float advance_ms);
Fixture *World_create_fixture(World *world);
PhysBox World_floor_box(World *world);

extern Object WorldProto;

#endif
