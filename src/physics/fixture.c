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

    fixture->restitution = 0.2;
    fixture->mass = 100;
    fixture->moment_of_inertia =
        fixture->mass * (pow(fixture->height, 2) + pow(fixture->width, 2)) / 12;
    fixture->drag = 0.47;
    fixture->surface_area = pow(fixture->width, 2);

    PhysPoint spring = {0, 0};
    fixture->spring = spring;

    fixture->on_ground = 0;

    return 1;
error:
    return 0;
}

void Fixture_calc_moment_of_inertia(Fixture *fixture) {
    fixture->moment_of_inertia =
        fixture->mass * (pow(fixture->height, 2) + pow(fixture->width, 2)) / 12;
}

void Fixture_set_wh(Fixture *fixture, double w, double h) {
    fixture->width = w;
    fixture->height = h;
    Fixture_calc_moment_of_inertia(fixture);
}

void Fixture_set_mass(Fixture *fixture, double m) {
    fixture->mass = m;
    Fixture_calc_moment_of_inertia(fixture);
}

void Fixture_set_rotation_degrees(Fixture *fixture, double degrees) {
    double rads = degrees * M_PI / 180;
    fixture->rotation_radians = rads;
}

double Fixture_rotation_degrees(Fixture *fixture) {
    return fixture->rotation_radians * 180 / M_PI;
}

void Fixture_control(Fixture *fixture, Controller *controller) {
    check_mem(fixture);
    if (controller == NULL) return;

    fixture->input_acceleration.x = 0;
    if (controller->dpad & CONTROLLER_DPAD_RIGHT) {
        if (!fixture->on_ground) {
            fixture->input_acceleration.x = MVMT_AIR_ACCEL;
            return;
        }

        if (fixture->velocity.x < 0) {
            fixture->input_acceleration.x = MVMT_TURN_ACCEL;
        } else {
            fixture->input_acceleration.x = MVMT_RUN_ACCEL;
        }
    }
    if (controller->dpad & CONTROLLER_DPAD_LEFT) {
        if (!fixture->on_ground) {
            fixture->input_acceleration.x = -1 * MVMT_AIR_ACCEL;
            return;
        }

        if (fixture->velocity.x > 0) {
            fixture->input_acceleration.x = -1 * MVMT_TURN_ACCEL;
        } else {
            fixture->input_acceleration.x = -1 * MVMT_RUN_ACCEL;
        }
    }
    if (controller->jump) {
        if (fixture->on_ground) {
            fixture->velocity.y = MVMT_JUMP_VELO_HI;
        }
    } else {
        if (fixture->velocity.y < MVMT_JUMP_VELO_LO) {
            fixture->velocity.y = MVMT_JUMP_VELO_LO;
        }
    }
    return;
error:
    return;
}

void Fixture_solve(Physics *physics, Fixture *fixture, double advance_ms) {
    check_mem(physics);
    check_mem(fixture);
    PhysPoint center = {fixture->x, fixture->y};
    PhysBox real_box = Fixture_real_box(fixture);
    World *world = fixture->world;

    PhysPoint f = {0,0};
    double torque = 0;
    double dt = fixture->time_scale * advance_ms / 1000.0;

    // PROACT
    // Start velocity verlet
    PhysPoint displacement = PhysPoint_scale(fixture->velocity, dt);
    displacement = PhysPoint_add(displacement,
            PhysPoint_scale(fixture->acceleration, 0.5 * dt * dt));

    // TODO: Make center a PhysPoint
    fixture->x += displacement.x;
    fixture->y += displacement.y;
    real_box = PhysBox_move(real_box, displacement);
    center = PhysBox_center(real_box);

    // INTERACT
    PhysPoint mtv = {0, 0};
    PhysBox floor_box = World_floor_box(world);
    int hit_floor = PhysBox_collision(real_box, floor_box, &mtv);
    PhysPoint gravity = {0, world->gravity * fixture->mass};
    f = PhysPoint_add(f, gravity);
    fixture->on_ground = hit_floor;
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
        double wai = fixture->angular_velocity;
        PhysPoint r = PhysPoint_subtract(center, poc);
        PhysPoint perp_norm = PhysPoint_perp(r);
        PhysPoint rot_v = PhysPoint_scale(PhysPoint_normalize(perp_norm), wai);
        PhysPoint vap = PhysPoint_add(vai, rot_v);

        // TODO: Bodies colliding with each OTHER (poop)
        double impulse_magnitude = -(1 + fixture->restitution) *
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

    double damping = -1;
    f = PhysPoint_add(f, PhysPoint_scale(fixture->velocity, damping));

    // Finish velocity verlet
    PhysPoint new_a = PhysPoint_scale(f, 1 / fixture->mass);

    // CONTROL
    Fixture_control(fixture, fixture->controller);
    if (fabs(fixture->velocity.x) >= MVMT_MAX_VELO &&
            sign(fixture->velocity.x) == sign(fixture->input_acceleration.x)) {
        fixture->input_acceleration.x = 0;
    }
    new_a = PhysPoint_add(new_a, fixture->input_acceleration);

    // REACT
    PhysPoint avg_a = PhysPoint_scale(
            PhysPoint_add(new_a, fixture->acceleration), 0.5);
    fixture->velocity = PhysPoint_add(fixture->velocity,
            PhysPoint_scale(avg_a, dt));

    if (fixture->input_acceleration.x == 0 && fixture->on_ground) {
        //TODO: proper friction
        int dir = sign(fixture->velocity.x);
        fixture->velocity.x -= dir * MVMT_FRICTION * dt;
        fixture->velocity.x = dir > 0 ? MAX(fixture->velocity.x, 0) :
            MIN(fixture->velocity.x, 0);
    }

    fixture->acceleration = avg_a;

    double angular_damping = -7;
    torque += fixture->angular_velocity * angular_damping;
    fixture->angular_acceleration = torque / fixture->moment_of_inertia;
    fixture->angular_velocity += fixture->angular_acceleration * dt;
    double delta_theta = fixture->angular_velocity * dt;
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

