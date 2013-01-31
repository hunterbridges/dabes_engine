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

    fixture->x_last_accel = 0;
    fixture->y_last_accel = 0;

    fixture->time_scale = 1.0;

    fixture->restitution = -0.5;
    fixture->mass = 100;
    fixture->drag = 0.47;
    fixture->surface_area = pow(fixture->width, 2);

    return 1;
error:
    return 0;
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
    World *world = fixture->world;

    float fy = 0;

    // Weight
    if (bounding_box.bl.y < world->height) {
        fy += fixture->mass * world->gravity;
    }

    // Air resistance
    //fy += -1 * 0.5 * world->air_density * fixture->drag * fixture->surface_area * pow(fixture->y_velo, 2);

    float dt = fixture->time_scale * advance_ms / 1000.0;

    float dy = fixture->y_velo * dt + (0.5 * fixture->y_last_accel * pow(dt, 2));

    fixture->y += dy;

    float new_ay = fy / fixture->mass;
    float avg_ay = 0.5 * (new_ay + fixture->y_last_accel);
    fixture->y_velo += avg_ay * dt;
    fixture->y_last_accel = avg_ay;

    if (bounding_box.bl.y > world->height && fixture->y_velo > 0) {
        fixture->y = PhysBox_center(bounding_box).y - 0.01;
        fixture->y_velo *= fixture->restitution;
    } else {
        float degs = Fixture_rotation_degrees(fixture);
        degs += 2 * dt * 100;
        Fixture_set_rotation_degrees(fixture, degs);
        while (fixture->rotation_radians > 2 * M_PI)
            fixture->rotation_radians -= 2 * M_PI;
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

