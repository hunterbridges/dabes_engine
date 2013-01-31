#include "world.h"

int World_init(void *self) {
    check_mem(self);

    World *world = self;
    world->num_fixtures = 0;
    world->time_scale = 1;
    world->width = SCREEN_WIDTH / DEFAULT_PPM;
    world->height = SCREEN_HEIGHT / DEFAULT_PPM;
    world->pixels_per_meter = DEFAULT_PPM;

    return 1;
error:
    return 0;
}

void World_destroy(void *self) {
    check_mem(self);

    free(self);
    return;
error:
    free(self);
}

void World_solve(Physics *physics, World *world, float advance_ms) {
    advance_ms *= world->time_scale;

    uint i = 0;
    for (i = 0; i < world->num_fixtures; i++) {
        Fixture *fixture = world->fixtures[i];
        Fixture_solve(physics, fixture, advance_ms);
    }
}

Fixture *World_create_fixture(World *world) {
    check_mem(world);

    Fixture *fixture = NEW(Fixture, "New fixture");
    world->fixtures[world->num_fixtures] = fixture;
    fixture->world = world;

    world->num_fixtures++;
    return fixture;
error:
    return NULL;
}

Object WorldProto = {
    .init = World_init,
    .destroy = World_destroy
};
