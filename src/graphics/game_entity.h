#ifndef __game_entity_h
#define __game_entity_h
#include <chipmunk/chipmunk.h>
#include "../prefix.h"
#include "../graphics/sprite.h"
#include "../physics/fixture.h"
#include "../input/input.h"
#include "../input/controller.h"

typedef enum {
    GameEntityPhysicsShapeTypeFixture = 0,
    GameEntityPhysicsShapeTypeCPShape
} GameEntityPhysicsShapeType;

typedef struct {
  void *engine;
  void *entity;
  void *scene;
  int on_ground;
} GameEntityStateData;

typedef struct GameEntity {
    Object proto;
    Controller *controller;
    uint32_t color;
    GLfloat alpha;
    GameEntityStateData *state;

    Sprite *sprite;
    int current_frame;

    struct {
        union {
            Fixture *fixture;
            cpShape *shape;
        };
        GameEntityPhysicsShapeType shape_type;
    } physics_shape;

    struct {
        GfxSize size;
        VPoint center;
        double mass;
        double rotation; // radians
        double edge_friction;
    } config;
} GameEntity;

struct Engine;
GameEntity *GameEntity_create();
void GameEntity_destroy(GameEntity *entity);
void GameEntity_render(GameEntity *self, void *engine);
void GameEntity_assign_controller(GameEntity *entity, Controller *controller);
void GameEntity_control(GameEntity *entity, struct Engine *engine);
VPoint GameEntity_center(GameEntity *entity);
VRect GameEntity_real_rect(GameEntity *entity);
VRect GameEntity_bounding_rect(GameEntity *entity);

extern Object GameEntityProto;

#endif
