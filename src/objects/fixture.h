#ifndef __fixture_h
#define __fixture_h
#include "../prefix.h"
#include "physics.h"

typedef struct Fixture {
    void *world;
    float x;  // X CENTER
    float y;  // Y CENTER
    float width;
    float height;
    float x_velo;
    float y_velo;
    float time_scale;
    float rotation;
} Fixture;

int Fixture_init(void *self);
void Fixture_solve(Physics *physics, Fixture *fixture, float advance_ms);
PhysBox Fixture_bounding_box(Fixture *fixture);
PhysBox Fixture_base_box(Fixture *fixture);
SDL_Rect Fixture_display_rect(Fixture *fixture);

extern Object FixtureProto;

#endif
