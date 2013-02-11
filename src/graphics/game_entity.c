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

void GameEntity_control(GameEntity *entity, Input *input) {
    check_mem(entity);
    if (entity->controller == NULL) return;
    if (entity->fixture == NULL) return;

    entity->fixture->input_acceleration.x = 0;
    if (entity->controller->dpad & CONTROLLER_DPAD_RIGHT)
        entity->fixture->input_acceleration.x = 90;
    if (entity->controller->dpad & CONTROLLER_DPAD_LEFT)
        entity->fixture->input_acceleration.x = -90;

    return;
error:
    return;
}

Object GameEntityProto = {
    .destroy = GameEntity_destroy,
    .init = GameEntity_init,
};

