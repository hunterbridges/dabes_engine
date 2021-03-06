#ifndef __entity_h
#define __entity_h
#include <chipmunk/chipmunk.h>
#include "../prefix.h"
#include "../graphics/sprite.h"
#include "../physics/fixture.h"
#include "../input/input.h"
#include "../input/controller.h"
#include "body.h"

typedef enum {
    EntityPhysicsShapeTypeFixture = 0,
    EntityPhysicsShapeTypeCPShape
} EntityPhysicsShapeType;

struct Scene;
typedef struct Entity {
    Controller *controller;
    short int auto_control;
    short int force_keyframe;

    Sprite *sprite;
    Body *body;
    struct Scene *scene;
    VVector4 bg_color;
    GLfloat alpha;

    int pixels_per_meter;

    float z;
  
    uint32_t timestamp;
    uint16_t add_index;
    uint64_t ukey;

    VPoint center;
    GfxSize size;

    int selected;
} Entity;

struct Engine;
struct DrawBuffer;

Entity *Entity_create(struct Engine *engine);
void Entity_destroy(Entity *entity);
void Entity_render(Entity *self, struct Engine *engine,
                   struct DrawBuffer *draw_buffer);
void Entity_assign_controller(Entity *entity, Controller *controller);
void Entity_update(Entity *entity, struct Engine *engine);
VPoint Entity_center(Entity *entity);
VRect Entity_real_rect(Entity *entity);
VRect Entity_bounding_rect(Entity *entity);
void Entity_set_add_index(Entity *entity, uint16_t add_index);
int Entity_cmp(void *a, void *b);

int Entity_set_center(Entity *entity, VPoint center);
int Entity_set_size(Entity *entity, GfxSize size);

#endif
