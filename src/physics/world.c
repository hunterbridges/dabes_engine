#include "world.h"

int World_init(void *self) {
    check_mem(self);

    World *world = self;
    world->num_fixtures = 0;
    world->time_scale = 1;
    world->width = SCREEN_WIDTH / DEFAULT_PPM;
    world->height = SCREEN_HEIGHT / DEFAULT_PPM;
    world->pixels_per_meter = DEFAULT_PPM;

    world->gravity = 9.81; // Da earff
    world->air_density = 1.2;

    world->fixtures = List_create();

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

void World_solve(Physics *physics, World *world, double advance_ms) {
    advance_ms *= world->time_scale;

    {
    LIST_FOREACH(world->fixtures, first, next, current) {
        Fixture *fixture = current->value;
        Fixture_step_reset(physics, fixture, advance_ms);
        Fixture_step_displace(physics, fixture);
        Fixture_step_apply_environment(physics, fixture);
    }
    }

    // TODO: Broad phase collisions

    LIST_FOREACH(world->fixtures, first, next, current) {
        Fixture *fixture = current->value;
        Fixture_step_apply_forces(physics, fixture);
        Fixture_step_control(fixture, fixture->controller);
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

Object WorldProto = {
    .init = World_init,
    .destroy = World_destroy
};
