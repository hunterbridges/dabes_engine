#include "engine.h"
#include "game_entity.h"

SDL_Rect GameEntity_rect(void *self) {
    GameEntity *thing = (GameEntity *)self;
    SDL_Rect rect = {roundf(thing->x) - thing->width / 2.0,
                     roundf(thing->y) - thing->height /2.0,
                     thing->width, thing->height};
    return rect;
}

int GameEntity_init(void *self) {
    check_mem(self);
    GameEntity *thing = (GameEntity *)self;
    thing->x = 270;
    thing->y = 0;
    thing->width = 100;
    thing->height = 100;
    thing->time_scale = 1.0;
    thing->rotation = 0;

    thing->rect = GameEntity_rect;
    return 1;

error:
    return 0;
}

void GameEntity_calc_physics(void *self, void *engine, int ticks) {
    check_mem(self);
    GameEntity *thing = (GameEntity *)self;

    int xaccel = 0;
    int yaccel = 0;

    if (thing->y < 480 - thing->height) {
        yaccel = 5;
    }

    float dt = (ticks / 2) * thing->time_scale / 100.0;

    thing->xvelo += xaccel * dt;
    thing->yvelo += yaccel * dt;

    thing->x += thing->xvelo;
    thing->y += thing->yvelo;

    thing->xvelo += xaccel * dt;
    thing->yvelo += yaccel * dt;

    SDL_Rect bounding_box = GameEntity_bounding_box(thing);
    if (bounding_box.y + bounding_box.h >= SCREEN_HEIGHT) {
      thing->yvelo = 1;
      thing->y = ceil(SCREEN_HEIGHT - bounding_box.h/2);
      int mod90 = (int)thing->rotation % 90;
      int ddif = mod90 - 45;
      int direction = ddif / 45.0 > 0 ? 1 : -1;
      thing->rotation += direction * dt * 1000;
      if (mod90 >= 85 || mod90 <= 5)
          thing->rotation = 90 * (int)thing->rotation / 90;
    } else {
      thing->rotation += dt * 1000;
    }
error:
    return;
}

SDL_Rect GameEntity_bounding_box(GameEntity *entity) {
    SDL_Rect rect = entity->rect(entity);
    Point pivot = {
        entity->x,
        entity->y
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
        Point rotated = rotate_point(point, pivot, entity->rotation);
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
    SDL_Rect bounding = {entity->x - neww/2,
                         entity->y - newh/2,
                         neww,
                         newh};
    return bounding;
}

void GameEntity_render(void *self, void *engine) {
    GameEntity *thing = self;
    Graphics *graphics = ((Engine *)engine)->graphics;

    /*
    SDL_Rect bounding_box = GameEntity_bounding_box(thing);
    GLfloat bcolor[3] = {1.f, 0.f, 0.f};
    graphics->draw_rect(graphics, bounding_box, bcolor, 0, 0);
    */

    SDL_Rect rect = thing->rect(thing);
    GLfloat color[3] = {0.f, 0.f, 0.f};
    graphics->draw_rect(graphics, rect, color, 0, thing->rotation);
}

Object GameEntityProto = {
   .init = GameEntity_init,
   .calc_physics = GameEntity_calc_physics,
   .render = GameEntity_render
};

