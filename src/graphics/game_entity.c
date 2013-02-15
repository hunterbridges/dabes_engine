#include "../core/engine.h"
#include "game_entity.h"

int GameEntity_init(void *self) {
    check_mem(self);
    GameEntity *entity = (GameEntity *)self;
    entity->fixture = NULL;
    entity->controller = NULL;
    entity->alpha = 1.f;
    entity->texture = 0;

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

void GameEntity_render(GameEntity *self, void *engine) {
    GameEntity *entity = self;
    Graphics *graphics = ((Engine *)engine)->graphics;

    GfxRect rect = Fixture_display_rect(entity->fixture);
    GLdouble color[4] = {0.f, 0.f, 0.f, entity->alpha};
    float degrees = Fixture_rotation_degrees(entity->fixture);
    glUseProgram(graphics->shader);

    Graphics_draw_rect(graphics, rect, color, entity->texture, degrees);
    glUseProgram(0);
}

void GameEntity_assign_controller(GameEntity *entity, Controller *controller) {
    check_mem(entity);
    entity->controller = controller;
    entity->fixture->controller = controller;
    return;
error:
    return;
}

void GameEntity_control(GameEntity *entity, Input *input) {
    check_mem(entity);
    check_mem(input);
    if (entity->controller == NULL) return;
    if (entity->fixture == NULL) return;
    return;
error:
    return;
}

Object GameEntityProto = {
    .destroy = GameEntity_destroy,
    .init = GameEntity_init,
};

