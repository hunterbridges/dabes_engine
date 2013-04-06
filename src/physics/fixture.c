#include <math.h>
#include "fixture.h"
#include "world.h"

int Fixture_init(void *self) {
    check_mem(self);

    Fixture *fixture = self;
    VPoint center = {0,0};
    fixture->center = center;
    fixture->width = 1;
    fixture->height = 1;

    fixture->rotation_radians = 0;
    fixture->angular_velocity = 0;
    fixture->angular_acceleration = 0;

    VPoint velocity = {0,0};
    fixture->velocity = velocity;
    VPoint acceleration = {0,0};
    fixture->acceleration = acceleration;

    fixture->time_scale = 1.0;

    fixture->restitution = 0.2;
    fixture->mass = 100;
    fixture->moment_of_inertia =
        fixture->mass * (pow(fixture->height, 2) + pow(fixture->width, 2)) / 12;
    fixture->drag = 0.47;
    fixture->surface_area = pow(fixture->width, 2);

    fixture->on_ground = 0;
    fixture->moving = 0;

    return 1;
error:
    return 0;
}

void Fixture_destroy(void *self) {
    check(self != NULL, "No fixture to destroy");
    Fixture *fixture = self;
    if (fixture->collisions) List_destroy(fixture->collisions);
    free(fixture);
error:
    return;
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

VRect Fixture_base_box(Fixture *fixture) {
    VRect rect = {
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

VRect Fixture_real_box(Fixture *fixture) {
    VRect rect = Fixture_base_box(fixture);

    rect = VRect_rotate(rect, fixture->center, fixture->rotation_radians);
    return rect;
}

VRect Fixture_display_rect(Fixture *fixture) {
    VRect base_box = Fixture_base_box(fixture);
    World *c_world = fixture->world;
    VPoint tl = {
        base_box.tl.x * c_world->pixels_per_meter,
        base_box.tl.y * c_world->pixels_per_meter
    };
    VPoint tr = {
        base_box.tr.x * c_world->pixels_per_meter,
        base_box.tr.y * c_world->pixels_per_meter
    };
    VPoint bl = {
        base_box.bl.x * c_world->pixels_per_meter,
        base_box.bl.y * c_world->pixels_per_meter
    };
    VPoint br = {
        base_box.br.x * c_world->pixels_per_meter,
        base_box.br.y * c_world->pixels_per_meter
    };
    VRect rect = {
        .tl = tl,
        .tr = tr,
        .bl = bl,
        .br = br
    };
    return rect;
}

VPoint Fixture_display_center(Fixture *fixture) {
    if (fixture == NULL) {
      VPoint zero = {0,0};
      return zero;
    }
    World *c_world = fixture->world;
    VPoint center = {
        fixture->center.x * c_world->pixels_per_meter,
        fixture->center.y * c_world->pixels_per_meter
    };
    return center;
}

int Fixture_hit_box(Fixture *fixture, VRect wall_box, Fixture *collider) {
    check_mem(fixture);

    VRect real_box = Fixture_real_box(fixture);
    real_box = VRect_move(real_box, fixture->step_displacement);

    VPoint center = VPoint_add(fixture->center,
            fixture->step_displacement);

    VPoint mtv = {0, 0};
    int hit_wall = VRect_collision(real_box, wall_box, &mtv);
    if (!hit_wall) return 0;

    VPoint scaled_mtv = VPoint_scale(mtv, -1);
    VPoint other_center = VRect_center(wall_box);
    VPoint direction = VPoint_subtract(other_center, center);
    double dot = VPoint_dot(mtv, direction);
    if (dot >= 0) {
        fixture->step_displacement = VPoint_add(fixture->step_displacement,
                scaled_mtv);
        real_box = VRect_move(real_box, scaled_mtv);
        center = VPoint_add(center, scaled_mtv);
    }

    VPoint collision_normal =
        VRect_cnormal_from_mtv(real_box, wall_box, mtv);
    VPoint poc = VRect_poc(real_box, wall_box);

    VPoint vai = fixture->step_velocity;
    double wai = fixture->step_angular_velocity;
    VPoint r = VPoint_subtract(center, poc);
    VPoint perp_norm = VPoint_perp(r);
    VPoint rot_v = VPoint_scale(VPoint_normalize(perp_norm), wai);
    VPoint vap = VPoint_add(vai, rot_v);

    double elasticity = fixture->restitution;
    double imp_mag_denom = VPoint_dot(collision_normal, collision_normal) *
        (1 / fixture->mass) +
        (pow(VPoint_dot(perp_norm, collision_normal), 2) /
         fixture->moment_of_inertia);

    if (collider) {
        VPoint other_r = VPoint_subtract(poc, other_center);
        VPoint other_perp_norm = VPoint_perp(other_r);

        imp_mag_denom += collider->mass;
        imp_mag_denom += pow(VPoint_dot(other_perp_norm, collision_normal), 2) /
         collider->moment_of_inertia;
    }

    double impulse_magnitude = -(1 + elasticity) *
        VPoint_dot(vap, collision_normal);
    if (imp_mag_denom != 0.0) {
        impulse_magnitude /= imp_mag_denom;
    } else {
        impulse_magnitude = 0;
    }
    fixture->step_velocity = VPoint_add(vai,
            VPoint_scale(collision_normal,
                impulse_magnitude / fixture->mass));
    fixture->step_angular_velocity += VPoint_dot(perp_norm,
            VPoint_scale(collision_normal, impulse_magnitude)) /
        fixture->moment_of_inertia;

    return 1;
error:
    return 0;
}

void Fixture_step_reset(Physics *physics, Fixture *fixture, double advance_ms) {
    check_mem(physics);
    check_mem(fixture);
    fixture->step_dt = fixture->time_scale * advance_ms / 1000.0;
    VPoint blank = {0,0};

    fixture->step_force = blank;
    fixture->step_displacement = blank;
    fixture->step_torque = 0;
    fixture->step_velocity = fixture->velocity;
    fixture->step_acceleration = fixture->acceleration;
    fixture->step_rotation = fixture->rotation_radians;
    fixture->step_angular_velocity = fixture->angular_velocity;
    fixture->step_angular_acceleration = fixture->angular_acceleration;
    fixture->moving = 0;
    fixture->colliding = 0;

    if (fixture->collisions != NULL) {
        if (fixture->collisions->first) List_clear_destroy(fixture->collisions);
        else List_destroy(fixture->collisions);
        fixture->collisions = NULL;
    }
  
    if (fixture->walls != NULL) {
        if (fixture->walls->first) List_clear_destroy(fixture->walls);
        else List_destroy(fixture->walls);
        fixture->walls = NULL;
    }

    int i = 0;
    for (i = FIXTURE_HISTORY_LENGTH - 2; i >= 0; i--) {
        int last_frame = i + 1;
        VRect old = fixture->history[i];
        fixture->history[last_frame] = old;
        if (i == 0) {
            VRect empty = {{0,0},{0,0},{0,0},{0,0}};
            fixture->history[i] = empty;
        }
    }
    return;
error:
    return;
}

void Fixture_step_displace(Physics *physics, Fixture *fixture) {
    check_mem(physics);
    check_mem(fixture);

    double dt = fixture->step_dt;
    VPoint displacement = VPoint_scale(fixture->velocity, dt);
    displacement = VPoint_add(displacement,
            VPoint_scale(fixture->acceleration, 0.5 * dt * dt));
    fixture->step_displacement = displacement;
    fixture->history[0] = Fixture_real_box(fixture);
    if (!VRect_is_equal(&fixture->history[0], &fixture->history[1]))
        fixture->moving = 1;

    return;
error:
    return;
}

void Fixture_step_apply_environment(Physics *physics, Fixture *fixture) {
    check_mem(physics);
    check_mem(fixture);
    World *world = fixture->world;

    VPoint gravity = {0, world->gravity * fixture->mass};
    fixture->step_force = VPoint_add(fixture->step_force, gravity);

    //TODO: Figure out "On ground" status again
    fixture->on_ground = 0;
    VRect bounding = VRect_bounding_box(Fixture_real_box(fixture));
    VPoint ground_sensor = {
      .x = fixture->center.x,
      .y = fixture->center.y + 5.0 / world->pixels_per_meter +
               (bounding.bl.y - bounding.tr.y)
    };
    if (fixture->walls) {
      LIST_FOREACH(fixture->walls, first, next, current) {
          VRect *wall = current->value;
          Fixture_hit_box(fixture, *wall, NULL);
          fixture->on_ground = VRect_contains_point(*wall, ground_sensor);
      }
    }
  
    if (fixture->on_ground) {
        VPoint gravity = {0, world->gravity * fixture->mass};
        fixture->step_force = VPoint_subtract(fixture->step_force, gravity);
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
        VPoint_add(fixture->step_force,
                VPoint_scale(fixture->velocity, damping));
    VPoint input_force = VPoint_scale(fixture->input_acceleration,
            fixture->mass);
    fixture->step_force = VPoint_add(fixture->step_force, input_force);

    if (fixture->collisions) {
        LIST_FOREACH(fixture->collisions, first, next, current) {
            FixtureCollision *collision = current->value;
            VRect collider_box = VRect_move(collision->collider->history[0],
                    collision->collider->step_displacement);
            Fixture_hit_box(fixture, collider_box, collision->collider);
            /*
            collision->collider->step_force = VPoint_add(collision->collider->step_force,
                input_force);
                */
        }
    }

    if (fixture->on_ground) {
        World *world = fixture->world;
        VPoint gravity = {0, world->gravity * fixture->mass};
      
        // TODO: make sure this is actually right
        double mu = 0.42;
        double friction_mag = VPoint_magnitude(gravity) * mu;
        VPoint friction_force = VPoint_scale(fixture->velocity,
                -1 * friction_mag);
        fixture->step_force = VPoint_add(fixture->step_force,
                friction_force);
    }

    fixture->step_acceleration =
        VPoint_scale(fixture->step_force, 1 / fixture->mass);

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
    fixture->step_acceleration = VPoint_add(fixture->step_acceleration,
            fixture->input_acceleration);

    return;
error:
    return;
}

void Fixture_step_commit(Physics *physics, Fixture *fixture) {
    check_mem(physics);
    check_mem(fixture);

    VPoint avg_a = VPoint_scale(
            VPoint_add(fixture->step_acceleration,
                fixture->acceleration), 0.5);
    fixture->step_velocity = VPoint_add(fixture->step_velocity,
            VPoint_scale(avg_a, fixture->step_dt));

    if (fixture->input_acceleration.x == 0 && fixture->on_ground) {
        //TODO: proper friction
        /*
        int dir = sign(fixture->step_velocity.x);
        fixture->step_velocity.x -= dir * MVMT_FRICTION * fixture->step_dt;
        fixture->step_velocity.x = dir > 0 ? MAX(fixture->step_velocity.x, 0) :
            MIN(fixture->step_velocity.x, 0);
            */
    }

    fixture->center = VPoint_add(fixture->center,
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
    .init = Fixture_init,
    .destroy = Fixture_destroy
};

