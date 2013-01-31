#include "fixture.h"
#include "world.h"

int Fixture_init(void *self) {
    check_mem(self);

    Fixture *fixture = self;
    fixture->x = 0;
    fixture->y = 0;
    fixture->width = 100 / DEFAULT_PPM;
    fixture->height = 100 / DEFAULT_PPM;
    fixture->time_scale = 1.0;
    fixture->rotation = 0;

    return 1;
error:
    return 0;
}

void Fixture_solve(Physics *physics, Fixture *fixture, float advance_ms) {
    check_mem(physics);
    check_mem(fixture);
    PhysBox bounding_box = Fixture_bounding_box(fixture);
    World *world = fixture->world;

    float xaccel = 0;
    float yaccel = 0;

    if (bounding_box.y + bounding_box.h < world->height) {
        yaccel = 5;
    }

    float dt = (advance_ms / 2) * fixture->time_scale / 1000.0;

    fixture->x_velo += xaccel * dt;
    fixture->y_velo += yaccel * dt;

    fixture->x += fixture->x_velo;
    fixture->y += fixture->y_velo;

    fixture->x_velo += xaccel * dt;
    fixture->y_velo += yaccel * dt;

    if (bounding_box.y + bounding_box.h >= world->height) {
      fixture->y_velo = 0;
      fixture->y = world->height - bounding_box.h/2;

      int mod90 = (int)fixture->rotation % 90;
      int ddif = mod90 - 45;

      int direction = ddif / 45.0 > 0 ? 1 : -1;
      fixture->rotation += direction * dt * 100;
      if (mod90 >= 88 || mod90 <= 2)
          fixture->rotation = 90 * (int)fixture->rotation / 90;
    } else {
      fixture->rotation += 2 * dt * 10000;
      while (fixture->rotation > 360) fixture->rotation -= 360;
    }
error:
    return;
}

PhysBox Fixture_bounding_box(Fixture *fixture) {
    PhysBox rect = Fixture_base_box(fixture);
    Point pivot = {
        fixture->x,
        fixture->y
    };

    Point tl = {rect.x, rect.y};
    Point tr = {rect.x + rect.w, rect.y};
    Point br = {rect.x + rect.w, rect.y + rect.h};
    Point bl = {rect.x, rect.y + rect.h};

    Point points[4] = {tl, tr, br, bl};

    float min_x, max_x, min_y, max_y;

    int i = 0;
    for (i = 0; i < 4; i++) {
        Point point = points[i];
        Point rotated = rotate_point(point, pivot, fixture->rotation);
        if (i == 0) {
            min_x = rotated.x;
            max_x = rotated.x;
            min_y = rotated.y;
            max_y = rotated.y;
        }
        if (rotated.x < min_x) min_x = rotated.x;
        if (rotated.y < min_y) min_y = rotated.y;
        if (rotated.x > max_x) max_x = rotated.x;
        if (rotated.y > max_y) max_y = rotated.y;
    }

    float neww = max_x - min_x;
    float newh = max_y - min_y;
    PhysBox bounding = {fixture->x - neww/2,
                         fixture->y - newh/2,
                         neww,
                         newh};
    return bounding;
}

PhysBox Fixture_base_box(Fixture *fixture) {
    PhysBox rect = {fixture->x - fixture->width / 2.0,
                    fixture->y - fixture->height / 2.0,
                    fixture->width, fixture->height};
    return rect;
}

SDL_Rect Fixture_display_rect(Fixture *fixture) {
    PhysBox base_box = Fixture_base_box(fixture);
    World *c_world = fixture->world;
    SDL_Rect rect = {
        base_box.x * c_world->pixels_per_meter,
        base_box.y * c_world->pixels_per_meter,
        base_box.w * c_world->pixels_per_meter,
        base_box.h * c_world->pixels_per_meter
    };
    return rect;
}

Object FixtureProto = {
    .init = Fixture_init
};

