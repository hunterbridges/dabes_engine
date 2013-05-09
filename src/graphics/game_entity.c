#include "../core/engine.h"
#include "game_entity.h"
#include "../audio/sfx.h"
#include "../physics/world.h"

GameEntity *GameEntity_create() {
    GameEntity *entity = calloc(1, sizeof(GameEntity));
    check(entity != NULL, "Failed to create entity");

    entity->alpha = 1.f;

    return entity;
error:
    return NULL;
}

void GameEntity_destroy(GameEntity *entity) {
    check(entity != NULL, "No entity to destroy");
// TODO: Remove fixture
    free(entity);
    return;
error:
    return;
}

void GameEntity_render(GameEntity *self, void *engine) {
    GameEntity *entity = self;
    Graphics *graphics = ((Engine *)engine)->graphics;

    VRect rect = VRectZero;
    float degrees;
    GLfloat color[4] = {0.f, 0.f, 0.f, entity->alpha};
    switch (entity->physics_shape.shape_type) {
        case GameEntityPhysicsShapeTypeFixture:
            {
            Fixture *fixture = entity->physics_shape.fixture;
            rect = Fixture_display_rect(entity->physics_shape.fixture);
            if (fixture->colliding) {
                color[1] = 1.f;
                color[3] = 1.f;
            }
            degrees = Fixture_rotation_degrees(fixture);
            }
            break;

        case GameEntityPhysicsShapeTypeCPShape:
            {
              cpShape *shape = entity->physics_shape.shape;
              degrees = cpBodyGetAngle(shape->body) * 180.0 / M_PI;
              cpVect pos = cpBodyGetPos(shape->body);
              VPoint center = {pos.x, pos.y};
              int i = 0;
              for (i = 0; i < cpPolyShapeGetNumVerts(shape); i++) {
                cpVect vert = cpPolyShapeGetVert(shape, i);
                VPoint point = {vert.x, vert.y};
                point = VPoint_add(point, center);
                if (i == 0) rect.tl = point;
                else if (i == 1) rect.bl = point;
                else if (i == 2) rect.br = point;
                else if (i == 3) rect.tr = point;
              }
              rect = VRect_scale(rect, self->pixels_per_meter);
            }
            break;
    }

    Graphics_draw_sprite(graphics, entity->sprite, rect, color, degrees);
}

VPoint GameEntity_center(GameEntity *entity) {
    switch (entity->physics_shape.shape_type) {
        case GameEntityPhysicsShapeTypeFixture:
            {
                Fixture *fixture = entity->physics_shape.fixture;
                if (!fixture) return VPointZero;
                return Fixture_display_center(fixture);
            }
            break;

        case GameEntityPhysicsShapeTypeCPShape:
            {
                cpShape *shape = entity->physics_shape.shape;
                cpVect pos = cpBodyGetPos(shape->body);
                VPoint center = {pos.x, pos.y};
                return VPoint_scale(center, entity->pixels_per_meter);
            }
            break;
    }
    return VPointZero;
}

void GameEntity_assign_controller(GameEntity *entity, Controller *controller) {
    check_mem(entity);
    entity->controller = controller;
    if (entity->physics_shape.shape_type ==GameEntityPhysicsShapeTypeFixture) {
        Fixture *fixture = entity->physics_shape.fixture;
        fixture->controller = controller;
    }
    return;
error:
    return;
}

void GameEntity_control(GameEntity *entity, Engine *engine) {
    Input *input = engine->input;
    (void)(input);
    if (entity->controller == NULL) return;

    switch (entity->physics_shape.shape_type) {
        case GameEntityPhysicsShapeTypeFixture:
            {
            }
            break;

        case GameEntityPhysicsShapeTypeCPShape:
            {
                cpShape *shape = entity->physics_shape.shape;
                cpBodyResetForces(shape->body);
                int on_ground = entity->state->on_ground;
                Controller *controller = entity->controller;
                cpVect step_velocity = cpBodyGetVel(shape->body);
                cpVect input_acceleration = cpvzero;
                input_acceleration.x = 0;
                if (controller->dpad & CONTROLLER_DPAD_RIGHT) {
                    if (on_ground) {
                        if (step_velocity.x < 0) {
                            input_acceleration.x = MVMT_TURN_ACCEL;
                        } else {
                            input_acceleration.x = MVMT_RUN_ACCEL;
                        }
                    } else {
                        input_acceleration.x = MVMT_AIR_ACCEL;
                    }
                }
                if (controller->dpad & CONTROLLER_DPAD_LEFT) {
                    if (on_ground) {
                        if (step_velocity.x > 0) {
                            input_acceleration.x = -1 * MVMT_TURN_ACCEL;
                        } else {
                            input_acceleration.x = -1 * MVMT_RUN_ACCEL;
                        }
                    } else {
                        input_acceleration.x = -1 * MVMT_AIR_ACCEL;
                    }
                }
                if (controller->jump) {
                    if (on_ground) {
                        Sfx *clomp = Audio_gen_sfx(engine->audio,
                                "media/sfx/jump.ogg");
                        Sfx_play(clomp);
                        step_velocity.y = MVMT_JUMP_VELO_HI;
                    }
                } else {
                    if (step_velocity.y < MVMT_JUMP_VELO_LO) {
                        step_velocity.y = MVMT_JUMP_VELO_LO;
                    }
                }

                if (fabs(step_velocity.x) >= MVMT_MAX_VELO &&
                        sign(step_velocity.x) ==
                            sign(input_acceleration.x)) {
                    input_acceleration.x = 0;
                }

                cpBodySetVel(shape->body, step_velocity);

                cpVect input_f =
                    cpvmult(input_acceleration, cpBodyGetMass(shape->body));
                cpVect app_point = {0, 0.1};
                cpBodyApplyForce(shape->body, input_f, app_point);
            }
            break;
    }
    return;
}

void GameEntity_derive_animation(GameEntity *entity, Engine *UNUSED(engine)) {
    VPoint velo = {0, 0};
    switch (entity->physics_shape.shape_type) {
        case GameEntityPhysicsShapeTypeFixture:
            {
                velo.x = entity->physics_shape.fixture->velocity.x;
                velo.y = entity->physics_shape.fixture->velocity.y;
            }
            break;

        case GameEntityPhysicsShapeTypeCPShape:
            {
                cpVect cp_velo =
                    cpBodyGetVel(entity->physics_shape.shape->body);
                velo.x = cp_velo.x;
                velo.y = cp_velo.y;
            }
            break;
    }

    float standing_thresh = 0.25;
    if (velo.x < -standing_thresh) {
        entity->sprite->direction = SPRITE_DIR_FACING_LEFT;
    } else if (velo.x > standing_thresh) {
        entity->sprite->direction = SPRITE_DIR_FACING_RIGHT;
    }

    if (velo.x > -standing_thresh && velo.x < standing_thresh) {
        Sprite_use_animation(entity->sprite, "standing");
    } else {
        Sprite_use_animation(entity->sprite, "running");
    }
}

void GameEntity_update(GameEntity *entity, Engine *engine) {
    check(entity != NULL, "Need entity to update entity");
    check(engine != NULL, "Need engine to update entity");
    GameEntity_control(entity, engine);
    GameEntity_derive_animation(entity, engine);
    Sprite_update(entity->sprite, engine);
error:
    return;
}

VRect GameEntity_real_rect(GameEntity *entity) {
    switch (entity->physics_shape.shape_type) {
        case GameEntityPhysicsShapeTypeFixture:
            {
                Fixture *fixture = entity->physics_shape.fixture;
                if (!fixture) return VRectZero;
                World *world = fixture->world;
                VRect freal = Fixture_real_box(fixture);
                return VRect_scale(freal, world->pixels_per_meter);
            }
            break;

        case GameEntityPhysicsShapeTypeCPShape:
            {
              VRect rect = VRectZero;
              cpShape *shape = entity->physics_shape.shape;
              float rads = cpBodyGetAngle(shape->body);
              cpVect pos = cpBodyGetPos(shape->body);
              VPoint center = {pos.x, pos.y};
              int i = 0;
              for (i = 0; i < cpPolyShapeGetNumVerts(shape); i++) {
                cpVect vert = cpPolyShapeGetVert(shape, i);
                VPoint point = {vert.x, vert.y};
                point = VPoint_add(point, center);
                if (i == 0) rect.tl = point;
                else if (i == 1) rect.bl = point;
                else if (i == 2) rect.br = point;
                else if (i == 3) rect.tr = point;
              }
              rect = VRect_rotate(rect, center, rads);
              rect = VRect_scale(rect, entity->pixels_per_meter);
              return rect;
            }
            break;
    }
    return VRectZero;
}

VRect GameEntity_bounding_rect(GameEntity *entity) {
    VRect real = GameEntity_real_rect(entity);
    return VRect_bounding_box(real);
}
