#include "engine.h"
#include "game_entity.h"

int GameEntity_init(void *self) {
    check_mem(self);
    GameEntity *entity = (GameEntity *)self;
    entity->fixture = NULL;

    return 1;

error:
    return 0;
}

void GameEntity_destroy(void *self) {
    check_mem(self);
    GameEntity *entity = (GameEntity *)self;
// TODO: Remove fixture
    free(entity);
    return;
error:
    return;
}

void GameEntity_render(void *self, void *engine) {
    GameEntity *entity = self;
    Graphics *graphics = ((Engine *)engine)->graphics;

    /*
    SDL_Rect bounding_box = GameEntity_bounding_box(thing);
    GLfloat bcolor[3] = {1.f, 0.f, 0.f};
    graphics->draw_rect(graphics, bounding_box, bcolor, 0, 0);
    */

    GfxRect rect = Fixture_display_rect(entity->fixture);
    GLfloat color[3] = {0.f, 0.f, 0.f};
    float degrees = Fixture_rotation_degrees(entity->fixture);
    Graphics_draw_rect(graphics, rect, color, 0, degrees);
}

Object GameEntityProto = {
    .destroy = GameEntity_destroy,
    .init = GameEntity_init,
    .render = GameEntity_render
};

