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

    VRect rect = Fixture_display_rect(entity->fixture);
    GLfloat color[4] = {0.f, 0.f, 0.f, entity->alpha};
    if (entity->fixture->colliding) {
        color[1] = 1.f;
        color[3] = 1.f;
    }
    float degrees = Fixture_rotation_degrees(entity->fixture);

    Graphics_draw_rect(graphics, rect, color, entity->texture, VPointZero,
                       entity->texture->size, degrees);
}

VPoint GameEntity_center(GameEntity *entity) {
    if (entity->fixture) {
        return Fixture_display_center(entity->fixture);
    } else {
        VPoint zero = {0, 0};
        return zero;
    }
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

