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

    fixture->restitution = 0.5;
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
    PhysPoint center = {fixture->x, fixture->y};
    PhysBox real_box = Fixture_real_box(fixture);
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

    PhysPoint mtv = {0, 0};
    PhysBox floor_box = World_floor_box(world);
    int hit_floor = PhysBox_collision(real_box, floor_box, &mtv);
    PhysPoint gravity = {0, world->gravity * fixture->mass};
    f = PhysPoint_add(f, gravity);
    if (hit_floor) {
        f = PhysPoint_subtract(f, gravity);
        real_box = PhysBox_move(real_box, PhysPoint_scale(mtv, -1));
        center = PhysBox_center(real_box);
        fixture->x = center.x;
        fixture->y = center.y;

        PhysPoint collision_normal = {0, -1};
        mtv.x = 0;
        mtv.y = 0;
        PhysPoint poc = PhysBox_poc(real_box, floor_box, mtv);

        PhysPoint vai = fixture->velocity;
        float wai = fixture->angular_velocity;
        PhysPoint r = PhysPoint_subtract(center, poc);
        PhysPoint perp_norm = PhysPoint_perp(r);
        PhysPoint rot_v = PhysPoint_scale(PhysPoint_normalize(perp_norm), wai);
        PhysPoint vap = PhysPoint_add(vai, rot_v);

        // TODO: Bodies colliding with each OTHER (poop)
        float impulse_magnitude = -(1 + fixture->restitution) *
            PhysPoint_dot(vap, collision_normal);
        impulse_magnitude /= PhysPoint_dot(collision_normal, collision_normal) *
            (1 / fixture->mass) +
            (pow(PhysPoint_dot(perp_norm, collision_normal), 2) /
             fixture->moment_of_inertia);
        fixture->velocity = PhysPoint_add(vai, PhysPoint_scale(collision_normal,
                    impulse_magnitude / fixture->mass));
        fixture->angular_velocity += PhysPoint_dot(perp_norm,
                PhysPoint_scale(collision_normal, impulse_magnitude)) / fixture->moment_of_inertia;
    }

    float damping = -1;
    //f = PhysPoint_add(f, PhysPoint_scale(fixture->velocity, damping));

    // Finish velocity verlet
    PhysPoint new_a = PhysPoint_scale(f, 1 / fixture->mass);
    PhysPoint avg_a = PhysPoint_scale(
            PhysPoint_add(new_a, fixture->acceleration), 0.5);
    fixture->velocity = PhysPoint_add(fixture->velocity,
            PhysPoint_scale(avg_a, dt));
    fixture->acceleration = avg_a;

    float angular_damping = -7;
    torque += fixture->angular_velocity * angular_damping;
    fixture->angular_acceleration = torque / fixture->moment_of_inertia;
    fixture->angular_velocity += fixture->angular_acceleration * dt;
    float delta_theta = fixture->angular_velocity * dt;
    fixture->rotation_radians += delta_theta;
    return;
error:
    exit(1);
    return;
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

PhysBox Fixture_real_box(Fixture *fixture) {
    PhysBox rect = Fixture_base_box(fixture);
    PhysPoint pivot = {
        fixture->x,
        fixture->y
    };

    rect = PhysBox_rotate(rect, pivot, fixture->rotation_radians);
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

