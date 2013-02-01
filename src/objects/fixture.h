#ifndef __fixture_h
#define __fixture_h
#include "../prefix.h"
#include "graphics.h"
#include "physics.h"

typedef struct Fixture {
    void *world;
    float x;  // X CENTER
    float y;  // Y CENTER
    float width;
    float height;

    float restitution;
    float mass;
    float drag;
    float surface_area;

    float rotation_radians;
    float angular_velocity;
    float angular_acceleration;
    float moment_of_inertia;

    PhysPoint velocity;
    PhysPoint acceleration;

    PhysPoint spring;

    float time_scale;
} Fixture;

void Fixture_set_rotation_degrees(Fixture *fixture, float degrees);
void Fixture_set_wh(Fixture *fixture, float w, float h);
void Fixture_set_mass(Fixture *fixture, float m);
float Fixture_rotation_degrees(Fixture *fixture);
int Fixture_init(void *self);
void Fixture_solve(Physics *physics, Fixture *fixture, float advance_ms);
PhysBox Fixture_bounding_box(Fixture *fixture);
PhysBox Fixture_base_box(Fixture *fixture);
GfxRect Fixture_display_rect(Fixture *fixture);

extern Object FixtureProto;

#endif
