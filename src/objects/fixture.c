#include "fixture.h"
#include "world.h"

int Fixture_init(void *self) {
    check_mem(self);

    Fixture *fixture = self;
    fixture->x = 0;
    fixture->y = 0;
    fixture->width = 100 / DEFAULT_PPM;
    fixture->height = 100 / DEFAULT_PPM;

    fixture->rotation_radians = 0;
    fixture->angular_velocity = 0;
    fixture->angular_acceleration = 0;

    PhysPoint velocity = {0,0};
    fixture->velocity = velocity;
    PhysPoint acceleration = {0,0};
    fixture->acceleration = acceleration;

    fixture->time_scale = 1.0;

    fixture->restitution = -0.5;
    fixture->mass = 100;
    fixture->moment_of_inertia =
        fixture->mass * (pow(fixture->height, 2) + pow(fixture->width, 2)) / 12;
    fixture->drag = 0.47;
    fixture->surface_area = pow(fixture->width, 2);

    PhysPoint spring = {0, 0};
    fixture->spring = spring;

    return 1;
error:
    return 0;
}

void Fixture_calc_moment_of_inertia(Fixture *fixture) {
    fixture->moment_of_inertia =
        fixture->mass * (pow(fixture->height, 2) + pow(fixture->width, 2)) / 12;
}

void Fixture_set_wh(Fixture *fixture, float w, float h) {
    fixture->width = w;
    fixture->height = h;
    Fixture_calc_moment_of_inertia(fixture);
}

void Fixture_set_mass(Fixture *fixture, float m) {
    fixture->mass = m;
    Fixture_calc_moment_of_inertia(fixture);
}

void Fixture_set_rotation_degrees(Fixture *fixture, float degrees) {
    float rads = degrees * M_PI / 180;
    fixture->rotation_radians = rads;
}

float Fixture_rotation_degrees(Fixture *fixture) {
    return fixture->rotation_radians * 180 / M_PI;
}

void Fixture_solve(Physics *physics, Fixture *fixture, float advance_ms) {
    check_mem(physics);
    check_mem(fixture);
    PhysBox bounding_box = Fixture_bounding_box(fixture);
    PhysPoint center = {fixture->x, fixture->y};
    PhysBox real_box = PhysBox_rotate(Fixture_base_box(fixture), center,
            fixture->rotation_radians);
    World *world = fixture->world;

    float stiffness = 10;

    PhysPoint f = {0,0};
    float torque = 0;
    float dt = fixture->time_scale * advance_ms / 1000.0;

    // Start velocity verlet
    PhysPoint displacement = PhysPoint_scale(fixture->velocity, dt);
    displacement = PhysPoint_add(displacement,
            PhysPoint_scale(fixture->acceleration, 0.5 * dt * dt));

    // TODO: Make center a PhysPoint
    fixture->x += displacement.x;
    fixture->y += displacement.y;
    real_box = PhysBox_move(real_box, displacement);
    center = PhysBox_center(real_box);

    if (bounding_box.bl.y < world->height) {
        PhysPoint gravity = {0, world->gravity * fixture->mass};
        f = PhysPoint_add(f, gravity);
    }

    // Spring thing
    PhysPoint spring_force = PhysPoint_subtract(real_box.tl, fixture->spring);
    spring_force = PhysPoint_scale(spring_force , -1 * stiffness);
    PhysPoint radius = PhysPoint_subtract(center, real_box.tl);
    float spring_xforce = PhysPoint_cross(radius, spring_force);

    torque += -1 * spring_xforce;
    f = PhysPoint_add(f, spring_force);

    float damping = -1;
    f = PhysPoint_add(f, PhysPoint_scale(fixture->velocity, damping));

    // Finish velocity verlet
    PhysPoint new_a = PhysPoint_scale(f, 1 / fixture->mass);
    PhysPoint avg_a = PhysPoint_scale(
            PhysPoint_add(new_a, fixture->acceleration), 0.5);
    fixture->velocity = PhysPoint_add(fixture->velocity,
            PhysPoint_scale(avg_a, dt));
    fixture->acceleration = avg_a;

    // TODO: angular damping
    float angular_damping = -7;
    torque += fixture->angular_velocity * angular_damping;
    fixture->angular_acceleration = torque / fixture->moment_of_inertia;
    fixture->angular_velocity += fixture->angular_acceleration * dt;
    float delta_theta = fixture->angular_acceleration * dt;
    fixture->rotation_radians += delta_theta;

    if (bounding_box.bl.y > world->height && fixture->velocity.y > 0) {
        fixture->y = PhysBox_center(bounding_box).y - 0.01;
        fixture->velocity.y *= fixture->restitution;
    }
error:
    return;
}

PhysBox Fixture_bounding_box(Fixture *fixture) {
    PhysBox rect = Fixture_base_box(fixture);
    PhysPoint pivot = {
        fixture->x,
        fixture->y
    };

    rect = PhysBox_rotate(rect, pivot, fixture->rotation_radians);

    float min_x, max_x, min_y, max_y;

    int i = 0;
    for (i = 0; i < 4; i++) {
        PhysPoint point = PhysBox_vertex(rect, i);
        if (i == 0) {
            min_x = point.x;
            max_x = point.x;
            min_y = point.y;
            max_y = point.y;
        }
        if (point.x < min_x) min_x = point.x;
        if (point.y < min_y) min_y = point.y;
        if (point.x > max_x) max_x = point.x;
        if (point.y > max_y) max_y = point.y;
    }

    PhysPoint tl = {min_x, min_y};
    PhysPoint tr = {max_x, min_y};
    PhysPoint br = {max_x, max_y};
    PhysPoint bl = {min_x, max_y};

    PhysBox bounding = {tl, tr, br, bl};
    return bounding;
}

PhysBox Fixture_base_box(Fixture *fixture) {
    PhysBox rect = {
        {fixture->x - fixture->width / 2.0,
        fixture->y - fixture->height / 2.0},

        {fixture->x + fixture->width / 2.0,
        fixture->y - fixture->height / 2.0},

        {fixture->x + fixture->width / 2.0,
        fixture->y + fixture->height / 2.0},

        {fixture->x - fixture->width / 2.0,
        fixture->y + fixture->height / 2.0}
    };
    return rect;
}

GfxRect Fixture_display_rect(Fixture *fixture) {
    PhysBox base_box = Fixture_base_box(fixture);
    World *c_world = fixture->world;
    GfxPoint tl = {
        base_box.tl.x * c_world->pixels_per_meter,
        base_box.tl.y * c_world->pixels_per_meter
    };
    GfxPoint tr = {
        base_box.tr.x * c_world->pixels_per_meter,
        base_box.tr.y * c_world->pixels_per_meter
    };
    GfxPoint bl = {
        base_box.bl.x * c_world->pixels_per_meter,
        base_box.bl.y * c_world->pixels_per_meter
    };
    GfxPoint br = {
        base_box.br.x * c_world->pixels_per_meter,
        base_box.br.y * c_world->pixels_per_meter
    };
    GfxRect rect = {
        .tl = tl,
        .tr = tr,
        .bl = bl,
        .br = br
    };
    return rect;
}

Object FixtureProto = {
    .init = Fixture_init
};

