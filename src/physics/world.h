#ifndef __world_h
#define __world_h
#include "../prefix.h"
#include "physics.h"
#include "fixture.h"
#include "world_grid.h"
#include "../graphics/tile_map.h"

struct World {
    double height;
    double width;
    double grid_size;
    double time_scale;
    double pixels_per_meter;
    double gravity;
    double air_density;
    uint num_fixtures;
    List *fixtures;
    WorldGrid *grid;
};
typedef struct World World;

World *World_create(int cols, int rows);
void World_destroy(World *world);
void World_solve(Physics *physics, World *world, TileMap *tile_map,
                 double advance_ms);
Fixture *World_create_fixture(World *world);
PhysBox World_floor_box(World *world);
PhysBox World_ceil_box(World *world);
PhysBox World_left_wall_box(World *world);
PhysBox World_right_wall_box(World *world);

#endif
