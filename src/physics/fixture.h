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

    double step_dt;
    PhysPoint step_force;
    double step_torque;
    PhysPoint step_displacement;
    PhysPoint step_velocity;
    PhysPoint step_acceleration;
    double step_rotation;
    double step_angular_velocity;
    double step_angular_acceleration;

    Controller *controller;
    PhysPoint input_acceleration;

    int on_ground;
    int moving;
    int colliding;

    PhysBox history[FIXTURE_HISTORY_LENGTH];
    List *collisions;
} Fixture;

int Fixture_init(void *self);
void Fixture_destroy(void *self);
void Fixture_set_rotation_degrees(Fixture *fixture, double degrees);
void Fixture_set_wh(Fixture *fixture, double w, double h);
void Fixture_set_mass(Fixture *fixture, double m);
double Fixture_rotation_degrees(Fixture *fixture);
PhysBox Fixture_base_box(Fixture *fixture);
PhysBox Fixture_real_box(Fixture *fixture);
GfxRect Fixture_display_rect(Fixture *fixture);

void Fixture_step_reset(Physics *physics, Fixture *fixture, double advance_ms);
void Fixture_step_displace(Physics *physics, Fixture *fixture);
void Fixture_step_apply_environment(Physics *physics, Fixture *fixture);
void Fixture_step_apply_forces(Physics *physics, Fixture *fixture);
void Fixture_step_control(Fixture *fixture, Controller *controller);
void Fixture_step_commit(Physics *physics, Fixture *fixture);

extern Object FixtureProto;

typedef struct FixtureCollision {
    PhysPoint poc;
    PhysPoint collision_normal;
    PhysPoint mtv;
    Fixture *collider;
} FixtureCollision;

#endif
