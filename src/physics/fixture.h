#ifndef __fixture_h
#define __fixture_h
#include "../prefix.h"
#include "../graphics/graphics.h"
#include "physics.h"

typedef struct Fixture {
    void *world;
    double x;  // X CENTER
    double y;  // Y CENTER
    double width;
    double height;

    double restitution;
    double mass;
    double drag;
    double surface_area;

    double rotation_radians;
    double angular_velocity;
    double angular_acceleration;
    double moment_of_inertia;

    PhysPoint velocity;
    PhysPoint acceleration;

    PhysPoint spring;

    double time_scale;
} Fixture;

void Fixture_set_rotation_degrees(Fixture *fixture, double degrees);
void Fixture_set_wh(Fixture *fixture, double w, double h);
void Fixture_set_mass(Fixture *fixture, double m);
double Fixture_rotation_degrees(Fixture *fixture);
int Fixture_init(void *self);
void Fixture_solve(Physics *physics, Fixture *fixture, double advance_ms);
PhysBox Fixture_base_box(Fixture *fixture);
PhysBox Fixture_real_box(Fixture *fixture);
GfxRect Fixture_display_rect(Fixture *fixture);

extern Object FixtureProto;

#endif
