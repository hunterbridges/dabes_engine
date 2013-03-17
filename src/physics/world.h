#ifndef __world_h
#define __world_h
#include <lcthw/liblcthw.h>
#include "../prefix.h"
#include "physics.h"
#include "fixture.h"

typedef struct World {
    Object proto;
    double height;
    double width;
    double time_scale;
    double pixels_per_meter;
    double gravity;
    double air_density;
    uint num_fixtures;
    List *fixtures;
} World;

int World_init(void *self);
void World_destroy(void *self);
void World_solve(Physics *physics, World *world, double advance_ms);
Fixture *World_create_fixture(World *world);
PhysBox World_floor_box(World *world);

extern Object WorldProto;

#endif
