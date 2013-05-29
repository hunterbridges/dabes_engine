#include "../core/engine.h"
#include "entity.h"
#include "../audio/sfx.h"
#include "../physics/world.h"

Entity *Entity_create() {
    Entity *entity = calloc(1, sizeof(Entity));
    check(entity != NULL, "Failed to create entity");

    entity->alpha = 1.f;

    return entity;
error:
    return NULL;
}

void Entity_destroy(Entity *entity) {
    check(entity != NULL, "No entity to destroy");

// TODO: Remove fixture

    free(entity);
    return;
error:
    return;
}

VPoint Entity_center(Entity *entity) {
    VPoint center =
        entity->body->_(gfx_center)(entity->body, entity->pixels_per_meter);
    return center;
}

VRect Entity_base_rect(Entity *entity) {
    Body *body = entity->body;
    return body->_(gfx_rect)(body, entity->pixels_per_meter, 0);
}

VRect Entity_real_rect(Entity *entity) {
    Body *body = entity->body;
    return body->_(gfx_rect)(body, entity->pixels_per_meter, 1);
}

VRect Entity_bounding_rect(Entity *entity) {
    VRect real = Entity_real_rect(entity);
    return VRect_bounding_box(real);
}

void Entity_render(Entity *self, void *engine) {
    Entity *entity = self;
    Graphics *graphics = ((Engine *)engine)->graphics;

    VRect rect = Entity_base_rect(self);
    float rads = entity->body->_(get_angle)(entity->body);
    float degrees = rads * 180.0 / M_PI;
    GLfloat color[4] = {0.f, 0.f, 0.f, entity->alpha};

    Graphics_draw_sprite(graphics, entity->sprite, rect, color, degrees);
}

void Entity_update(Entity *entity, Engine *engine) {
    check(entity != NULL, "Need entity to update entity");
    check(engine != NULL, "Need engine to update entity");

    Scripting_call_hook(engine->scripting, entity, "main");
    Sprite_update(entity->sprite, engine);
error:
    return;
}
