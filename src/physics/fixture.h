#ifndef __fixture_h
#define __fixture_h
#include "../prefix.h"
#include "../graphics/graphics.h"
#include "../input/controller.h"
#include "physics.h"

typedef struct Fixture {
    Object proto;
    void *world;
    double time_scale;

    PhysPoint center;
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

    Controller *controller;
    PhysPoint input_acceleration;

    int on_ground;
} Fixture;

void Fixture_set_rotation_degrees(Fixture *fixture, double degrees);
void Fixture_set_wh(Fixture *fixture, double w, double h);
void Fixture_set_mass(Fixture *fixture, double m);
double Fixture_rotation_degrees(Fixture *fixture);
int Fixture_init(void *self);
void Fixture_solve(Physics *physics, Fixture *fixture, double advance_ms);
void Fixture_control(Fixture *fixture, Controller *controller);
PhysBox Fixture_base_box(Fixture *fixture);
PhysBox Fixture_real_box(Fixture *fixture);
GfxRect Fixture_display_rect(Fixture *fixture);

extern Object FixtureProto;

#endif
