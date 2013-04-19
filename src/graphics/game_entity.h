#ifndef __game_entity_h
#define __game_entity_h
#include <chipmunk/chipmunk.h>
#include "../prefix.h"
#include "../physics/fixture.h"
#include "../input/input.h"
#include "../input/controller.h"

typedef enum {
    GameEntityPhysicsShapeTypeFixture = 0,
    GameEntityPhysicsShapeTypeCPShape
} GameEntityPhysicsShapeType;

typedef struct {
  int on_ground;
} GameEntityStateData;

typedef struct GameEntity {
    Object proto;
    struct {
        union {
            Fixture *fixture;
            cpShape *shape;
        };
        GameEntityPhysicsShapeType shape_type;
    } physics_shape;
    Controller *controller;
    uint32_t color;
    GLfloat alpha;
    GfxTexture *texture;
    GameEntityStateData *state;
} GameEntity;

int GameEntity_init(void *self);
void GameEntity_destroy(void *self);
void GameEntity_render(GameEntity *self, void *engine);
void GameEntity_assign_controller(GameEntity *entity, Controller *controller);
void GameEntity_control(GameEntity *entity, Input *input);
VPoint GameEntity_center(GameEntity *entity);
VRect GameEntity_real_rect(GameEntity *entity);
VRect GameEntity_bounding_rect(GameEntity *entity);

extern Object GameEntityProto;

#endif
