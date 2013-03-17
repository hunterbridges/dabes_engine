#include "fixture.h"
#include "world.h"

int Fixture_init(void *self) {
    check_mem(self);

    Fixture *fixture = self;
    PhysPoint center = {0,0};
    fixture->center = center;
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

PhysBox Fixture_base_box(Fixture *fixture) {
    PhysBox rect = {
        {fixture->center.x - fixture->width / 2.0,
        fixture->center.y - fixture->height / 2.0},

        {fixture->center.x + fixture->width / 2.0,
        fixture->center.y - fixture->height / 2.0},

        {fixture->center.x + fixture->width / 2.0,
        fixture->center.y + fixture->height / 2.0},

        {fixture->center.x - fixture->width / 2.0,
        fixture->center.y + fixture->height / 2.0}
    };
    return rect;
}

PhysBox Fixture_real_box(Fixture *fixture) {
    PhysBox rect = Fixture_base_box(fixture);

    rect = PhysBox_rotate(rect, fixture->center, fixture->rotation_radians);
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

int Fixture_hit_wall(Fixture *fixture, PhysBox wall_box) {
    check_mem(fixture);

    PhysBox real_box = Fixture_real_box(fixture);
    real_box = PhysBox_move(real_box, fixture->step_displacement);

    PhysPoint center = PhysPoint_add(fixture->center,
            fixture->step_displacement);

    PhysPoint mtv = {0, 0};
    int hit_wall = PhysBox_collision(real_box, wall_box, &mtv);
    if (!hit_wall) return 0;

    PhysPoint scaled_mtv = PhysPoint_scale(mtv, -1);
    fixture->step_displacement = PhysPoint_add(fixture->step_displacement,
            scaled_mtv);
    real_box = PhysBox_move(real_box, scaled_mtv);
    center = PhysPoint_add(center, scaled_mtv);

    PhysPoint collision_normal = {0, -1};

    PhysPoint poc = PhysBox_poc(real_box, wall_box);

    PhysPoint vai = fixture->step_velocity;
    double wai = fixture->step_angular_velocity;
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
    fixture->step_velocity = PhysPoint_add(vai,
            PhysPoint_scale(collision_normal,
                impulse_magnitude / fixture->mass));
    fixture->step_angular_velocity += PhysPoint_dot(perp_norm,
            PhysPoint_scale(collision_normal, impulse_magnitude)) /
        fixture->moment_of_inertia;

    return 1;
error:
    return 0;
}

void Fixture_step_reset(Physics *physics, Fixture *fixture, double advance_ms) {
    check_mem(physics);
    check_mem(fixture);
    fixture->step_dt = fixture->time_scale * advance_ms / 1000.0;
    PhysPoint blank = {0,0};

    fixture->step_force = blank;
    fixture->step_displacement = blank;
    fixture->step_torque = 0;
    fixture->step_velocity = fixture->velocity;
    fixture->step_acceleration = fixture->acceleration;
    fixture->step_rotation = fixture->rotation_radians;
    fixture->step_angular_velocity = fixture->angular_velocity;
    fixture->step_angular_acceleration = fixture->angular_acceleration;

    return;
error:
    return;
}

void Fixture_step_displace(Physics *physics, Fixture *fixture) {
    check_mem(physics);
    check_mem(fixture);

    double dt = fixture->step_dt;
    PhysPoint displacement = PhysPoint_scale(fixture->velocity, dt);
    displacement = PhysPoint_add(displacement,
            PhysPoint_scale(fixture->acceleration, 0.5 * dt * dt));
    fixture->step_displacement = displacement;
    return;
error:
    return;
}

void Fixture_step_apply_environment(Physics *physics, Fixture *fixture) {
    check_mem(physics);
    check_mem(fixture);
    World *world = fixture->world;

    PhysPoint gravity = {0, world->gravity * fixture->mass};
    fixture->step_force = PhysPoint_add(fixture->step_force, gravity);

    PhysBox floor_box = World_floor_box(world);
    fixture->on_ground = Fixture_hit_wall(fixture, floor_box);
    if (fixture->on_ground) {
        PhysPoint gravity = {0, world->gravity * fixture->mass};
        fixture->step_force = PhysPoint_subtract(fixture->step_force, gravity);
    }

    return;
error:
    return;
}

void Fixture_step_apply_forces(Physics *physics, Fixture *fixture) {
    check_mem(physics);
    check_mem(fixture);
    double damping = -1; // I guess this is kg/s
    fixture->step_force =
        PhysPoint_add(fixture->step_force,
                PhysPoint_scale(fixture->velocity, damping));

    fixture->step_acceleration = PhysPoint_scale(fixture->step_force,
            1 / fixture->mass);
    return;
error:
    return;
}

void Fixture_step_control(Fixture *fixture, Controller *controller) {
    check_mem(fixture);
    if (controller == NULL) return;

    fixture->input_acceleration.x = 0;
    if (controller->dpad & CONTROLLER_DPAD_RIGHT) {
        if (fixture->on_ground) {
            if (fixture->step_velocity.x < 0) {
                fixture->input_acceleration.x = MVMT_TURN_ACCEL;
            } else {
                fixture->input_acceleration.x = MVMT_RUN_ACCEL;
            }
        } else {
            fixture->input_acceleration.x = MVMT_AIR_ACCEL;
        }
    }
    if (controller->dpad & CONTROLLER_DPAD_LEFT) {
        if (fixture->on_ground) {
            if (fixture->step_velocity.x > 0) {
                fixture->input_acceleration.x = -1 * MVMT_TURN_ACCEL;
            } else {
                fixture->input_acceleration.x = -1 * MVMT_RUN_ACCEL;
            }
        } else {
            fixture->input_acceleration.x = -1 * MVMT_AIR_ACCEL;
        }
    }
    if (controller->jump) {
        if (fixture->on_ground) {
            fixture->step_velocity.y = MVMT_JUMP_VELO_HI;
        }
    } else {
        if (fixture->step_velocity.y < MVMT_JUMP_VELO_LO) {
            fixture->step_velocity.y = MVMT_JUMP_VELO_LO;
        }
    }

    if (fabs(fixture->step_velocity.x) >= MVMT_MAX_VELO &&
            sign(fixture->step_velocity.x) ==
                sign(fixture->input_acceleration.x)) {
        fixture->input_acceleration.x = 0;
    }
    fixture->step_acceleration = PhysPoint_add(fixture->step_acceleration,
            fixture->input_acceleration);

    return;
error:
    return;
}

void Fixture_step_commit(Physics *physics, Fixture *fixture) {
    check_mem(physics);
    check_mem(fixture);

    PhysPoint avg_a = PhysPoint_scale(
            PhysPoint_add(fixture->step_acceleration,
                fixture->acceleration), 0.5);
    fixture->step_velocity = PhysPoint_add(fixture->step_velocity,
            PhysPoint_scale(avg_a, fixture->step_dt));

    if (fixture->input_acceleration.x == 0 && fixture->on_ground) {
        //TODO: proper friction
        int dir = sign(fixture->step_velocity.x);
        fixture->step_velocity.x -= dir * MVMT_FRICTION * fixture->step_dt;
        fixture->step_velocity.x = dir > 0 ? MAX(fixture->step_velocity.x, 0) :
            MIN(fixture->step_velocity.x, 0);
    }

    fixture->center = PhysPoint_add(fixture->center,
            fixture->step_displacement);
    fixture->velocity = fixture->step_velocity;
    fixture->acceleration = avg_a;

    double angular_damping = -7;
    fixture->step_torque += fixture->step_angular_velocity * angular_damping;
    fixture->angular_acceleration =
        fixture->step_torque / fixture->moment_of_inertia;
    fixture->angular_velocity =
        fixture->step_angular_velocity +
        fixture->angular_acceleration * fixture->step_dt;
    double delta_theta = fixture->angular_velocity * fixture->step_dt;
    fixture->rotation_radians = fixture->step_rotation + delta_theta;

    return;
error:
    return;
}

Object FixtureProto = {
    .init = Fixture_init
};
