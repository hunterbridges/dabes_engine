#include "world.h"

int World_init(void *self) {
    check_mem(self);

    World *world = self;
    world->num_fixtures = 0;
    world->time_scale = 1;
    world->width = SCREEN_WIDTH / DEFAULT_PPM;
    world->height = SCREEN_HEIGHT / DEFAULT_PPM;
    world->grid_size = PHYS_DEFAULT_GRID_SIZE;
    world->pixels_per_meter = DEFAULT_PPM;

    world->gravity = 9.81; // Da earff
    world->air_density = 1.2;

    world->fixtures = List_create();
    world->grid = WorldGrid_create(world->height / world->grid_size,
            world->width / world->grid_size,
            world->grid_size);
    check(world->grid != NULL, "Couldn't create WorldGrid");

    return 1;
error:
    return 0;
}

void World_destroy(void *self) {
    check_mem(self);
    World *world = self;

    LIST_FOREACH(world->fixtures, first, next, current) {
        Fixture *fixture = current->value;
        fixture->_(destroy)(fixture);
    }

    List_destroy(world->fixtures);
    free(self);
    return;
error:
    free(self);
}

void World_collide_fixture(World *world, Fixture *fixture) {
    List *near = WorldGrid_members_near_fixture(world->grid, fixture);
    if (!near) return;
    LIST_FOREACH(near, first, next, current) {
        WorldGridMember *member = current->value;
        if (member->member_type == WORLDGRIDMEMBER_FIXTURE) {
            PhysPoint mtv = {0,0};
            //PhysBox this_box = fixture->history[0];
            PhysBox this_box = Fixture_real_box(fixture);
            this_box = PhysBox_move(this_box, fixture->step_displacement);
            //PhysBox other_box = member->fixture->history[0];
            PhysBox other_box = Fixture_real_box(member->fixture);
            other_box = PhysBox_move(other_box, member->fixture->step_displacement);
            if (!PhysBox_collision(this_box, other_box, &mtv)) continue;
            if (fixture->collisions == NULL) {
                fixture->collisions = List_create();
            }

            FixtureCollision *collision = calloc(1, sizeof(FixtureCollision));
            assert(collision != NULL);
            collision->collider = member->fixture;
            collision->mtv = mtv;
            collision->collision_normal = PhysBox_cnormal_from_mtv(
                    this_box,
                    other_box,
                    mtv);
            List_push(fixture->collisions, collision);
            fixture->colliding = 1;
        }
    }
    List_destroy(near);
}

void World_solve(Physics *physics, World *world, double advance_ms) {
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

PhysBox World_floor_box(World *world) {
    PhysBox floor = {
        {0, world->height},
        {world->width, world->height},
        {world->width, world->height * 2},
        {0, world->height * 2}
    };
    return floor;
}

PhysBox World_ceil_box(World *world) {
    PhysBox floor = {
        {0, -world->height},
        {world->width, -world->height},
        {world->width, 0},
        {0, 0}
    };
    return floor;
}

PhysBox World_left_wall_box(World *world) {
    PhysBox floor = {
        {-world->width, 0},
        {0, 0},
        {0, world->height},
        {-world->width, world->height}
    };
    return floor;
}

PhysBox World_right_wall_box(World *world) {
    PhysBox floor = {
        {world->width, 0},
        {world->width * 2, 0},
        {world->width * 2, world->height},
        {world->width, world->height}
    };
    return floor;
}

Object WorldProto = {
    .init = World_init,
    .destroy = World_destroy
};
