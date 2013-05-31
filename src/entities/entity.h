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

typedef struct Entity {
    Object proto;

    Controller *controller;
    Sprite *sprite;
    Body *body;
    GLfloat alpha;

    int pixels_per_meter;
} Entity;

struct Engine;
Entity *Entity_create();
void Entity_destroy(Entity *entity);
void Entity_render(Entity *self, void *engine);
void Entity_assign_controller(Entity *entity, Controller *controller);
void Entity_update(Entity *entity, struct Engine *engine);
VPoint Entity_center(Entity *entity);
VRect Entity_real_rect(Entity *entity);
VRect Entity_bounding_rect(Entity *entity);

extern Object EntityProto;

#endif
