#include "world.h"


World *World_create(int cols, int rows) {
    World *world = calloc(1, sizeof(World));
    check(world != NULL, "Couldn't create world");
  
    world->num_fixtures = 0;
    world->time_scale = 1;
    world->grid_size = PHYS_DEFAULT_GRID_SIZE;
    world->pixels_per_meter = DEFAULT_PPM;
    world->width = cols * world->grid_size;
    world->height = rows * world->grid_size;

    world->gravity = 9.81; // Da earff
    world->air_density = 1.2;

    world->fixtures = List_create();
    world->grid = WorldGrid_create(rows, cols, world->grid_size);
    check(world->grid != NULL, "Couldn't create WorldGrid");

    return world;
error:
    if (world) free(world);
    return NULL;
}

void World_destroy(World *world) {
    check(world != NULL, "No world to destroy");
  
    LIST_FOREACH(world->fixtures, first, next, current) {
        Fixture *fixture = current->value;
        fixture->_(destroy)(fixture);
    }

    List_destroy(world->fixtures);
    free(world);
    return;
error:
    return;
}

void World_collide_tiles(World *world, Fixture *fixture, TileMap *tile_map) {
    List *cells = WorldGrid_cells_for_box(world->grid,
                                          Fixture_real_box(fixture));
    if (!cells) return;
    LIST_FOREACH(cells, first, next, current) {
        WorldGridCell *cell = current->value;
        int gid_idx = cell->row * tile_map->cols + cell->col;
        if (gid_idx > tile_map->cols * tile_map->rows - 1) continue;
        TileMapLayer *base_layer = DArray_get(tile_map->layers, 0);
        uint32_t gid = base_layer->tile_gids[gid_idx];
        if (gid == 0) continue;
        if (fixture->walls == NULL) {
            fixture->walls = List_create();
        }
        VRect *wall = calloc(1, sizeof(VRect));
        *wall = WorldGrid_box_for_cell(world->grid, cell->col, cell->row);
        List_push(fixture->walls, wall);
    }
}

void World_collide_fixture(World *world, Fixture *fixture) {
    List *near = WorldGrid_members_near_fixture(world->grid, fixture);
    if (!near) return;
    LIST_FOREACH(near, first, next, current) {
        WorldGridMember *member = current->value;
        if (member->member_type == WORLDGRIDMEMBER_FIXTURE) {
            VPoint mtv = {0,0};
            //VRect this_box = fixture->history[0];
            VRect this_box = Fixture_real_box(fixture);
            this_box = VRect_move(this_box, fixture->step_displacement);
            //VRect other_box = member->fixture->history[0];
            VRect other_box = Fixture_real_box(member->fixture);
            other_box = VRect_move(other_box,
                                     member->fixture->step_displacement);
            if (!VRect_collision(this_box, other_box, &mtv)) continue;
            if (fixture->collisions == NULL) {
                fixture->collisions = List_create();
            }

            FixtureCollision *collision = calloc(1, sizeof(FixtureCollision));
            assert(collision != NULL);
            collision->collider = member->fixture;
            collision->mtv = mtv;
            collision->collision_normal = VRect_cnormal_from_mtv(
                    this_box,
                    other_box,
                    mtv);
            List_push(fixture->collisions, collision);
            fixture->colliding = 1;
        }
    }
    List_destroy(near);
}

void World_solve(Physics *physics, World *world, TileMap *tile_map,
                 double advance_ms) {
    advance_ms *= world->time_scale;

    int fixtures_updated = 0;
    {
    LIST_FOREACH(world->fixtures, first, next, current) {
        Fixture *fixture = current->value;
        Fixture_step_reset(physics, fixture, advance_ms);

        Fixture_step_displace(physics, fixture);
        if (fixture->moving) {
            WorldGrid_update_fixture(world->grid, fixture);
            fixtures_updated++;
        }

        World_collide_tiles(world, fixture, tile_map);
        Fixture_step_apply_environment(physics, fixture);
    }
    }

    // debug("%d fixtures updated", fixtures_updated);
    // TODO: Broad phase collisions

    LIST_FOREACH(world->fixtures, first, next, current) {
        Fixture *fixture = current->value;
        World_collide_fixture(world, fixture);
        Fixture_step_control(fixture, fixture->controller);
        Fixture_step_apply_forces(physics, fixture);
        Fixture_step_commit(physics, fixture);
    }
}

Fixture *World_create_fixture(World *world) {
    check_mem(world);

    Fixture *fixture = NEW(Fixture, "New fixture");
    List_push(world->fixtures, fixture);
    fixture->world = world;

    world->num_fixtures++;
    return fixture;
error:
    return NULL;
}

VRect World_floor_box(World *world) {
    VRect floor = {
        {0, world->height},
        {world->width, world->height},
        {world->width, world->height * 2},
        {0, world->height * 2}
    };
    return floor;
}

VRect World_ceil_box(World *world) {
    VRect floor = {
        {0, -world->height},
        {world->width, -world->height},
        {world->width, 0},
        {0, 0}
    };
    return floor;
}

VRect World_left_wall_box(World *world) {
    VRect floor = {
        {-world->width, 0},
        {0, 0},
        {0, world->height},
        {-world->width, world->height}
    };
    return floor;
}

VRect World_right_wall_box(World *world) {
    VRect floor = {
        {world->width, 0},
        {world->width * 2, 0},
        {world->width * 2, world->height},
        {world->width, world->height}
    };
    return floor;
}

