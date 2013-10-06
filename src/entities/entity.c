#include <lcthw/darray_algos.h>
#include "../core/engine.h"
#include "entity.h"
#include "../audio/sfx.h"
#include "../physics/world.h"
#include "../scenes/scene.h"

Entity *Entity_create(Engine *engine) {
    Entity *entity = calloc(1, sizeof(Entity));
    check(entity != NULL, "Failed to create entity");

    entity->alpha = 1.f;
    VVector4 bg_color = {.raw = {0, 0, 0, 0}};
    entity->bg_color = bg_color;
    entity->timestamp = (uint32_t)Engine_get_ticks(engine);
    entity->z = -100.f;

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
    if (body) {
      return body->_(gfx_rect)(body, entity->pixels_per_meter, 0);
    } else {
      VRect base_rect =
          VRect_from_xywh(entity->center.x - entity->size.w / 2.0 * entity->pixels_per_meter,
                          entity->center.y - entity->size.h / 2.0 * entity->pixels_per_meter,
                          entity->size.w * entity->pixels_per_meter,
                          entity->size.h * entity->pixels_per_meter);
      return base_rect;
    }
}

VRect Entity_real_rect(Entity *entity) {
    Body *body = entity->body;
    if (body) {
      return body->_(gfx_rect)(body, entity->pixels_per_meter, 1);
    } else {
      VRect base_rect =
          VRect_from_xywh(entity->center.x - entity->size.w / 2.0,
                          entity->center.y - entity->size.h / 2.0,
                          entity->size.w,
                          entity->size.h);
      // TODO: Rotate
      return base_rect;
    }
}

VRect Entity_bounding_rect(Entity *entity) {
    VRect real = Entity_real_rect(entity);
    return VRect_bounding_box(real);
}

void Entity_render(Entity *entity, struct Engine *engine,
                   struct DrawBuffer *draw_buffer) {
    Graphics *graphics = engine->graphics;

    VRect rect = Entity_base_rect(entity);
    float rads = 0;
    if (entity->body) {
      rads = entity->body->_(get_angle)(entity->body);
    }
    float degrees = rads * 180.0 / M_PI;

    Graphics_draw_sprite(graphics, entity->sprite, draw_buffer, rect,
                         entity->bg_color.raw, degrees, entity->alpha,
                         entity->z);
}

void Entity_update(Entity *entity, Engine *engine) {
    check(entity != NULL, "Need entity to update entity");
    check(engine != NULL, "Need engine to update entity");

    if (entity->body && entity->body->cp_body) {
      cpBodyResetForces(entity->body->cp_body);
    }
    Scripting_call_hook(engine->scripting, entity, "main");
    Sprite_update(entity->sprite, engine);
error:
    return;
}

void Entity_refresh_ukey(Entity *entity) {
    if (entity->scene) {
        BSTree_delete(entity->scene->entities, &entity->ukey);
    }
    entity->ukey = (((uint64_t)entity->timestamp << 16) |
                     entity->add_index);
    if (entity->scene) {
        BSTree_set(entity->scene->entities, &entity->ukey, entity);
    }
}

void Entity_set_add_index(Entity *entity, uint16_t add_index) {
    entity->add_index = add_index;
    Entity_refresh_ukey(entity);
}

int Entity_cmp(void *a, void *b) {
    uint64_t left = a ? *(uint64_t *)a : 0;
    uint64_t right = b ? *(uint64_t *)b : 0;
    if (left > right) return 1;
    if (left == right) return 0;
    if (left < right) return -1;
    return 0;
}

int Entity_set_center(Entity *entity, VPoint center) {
    if (entity->body) {
        entity->body->_(set_pos)(entity->body, center);
    } else {
        entity->center = center;
    }
    return 1;
}

int Entity_set_size(Entity *entity, GfxSize size) {
    if (entity->body) return 0;
    entity->size = size;
    return 1;
}
