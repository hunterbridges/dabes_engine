#include "../core/engine.h"
#include "game_entity.h"
#include "../physics/world.h"

int GameEntity_init(void *self) {
    check_mem(self);
    GameEntity *entity = (GameEntity *)self;
    entity->physics_shape.fixture = NULL;
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
              rect = VRect_scale(rect, DEFAULT_PPM);
            }
            break;
    }

    Graphics_draw_rect(graphics, rect, color, entity->texture, VPointZero,
                       entity->texture->size, degrees);
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
                return VPoint_scale(center, DEFAULT_PPM);
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

void GameEntity_control(GameEntity *entity, Input *input) {
    check_mem(entity);
    check_mem(input);
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
                cpBodyApplyForce(shape->body, input_f, cpvzero);
            }
            break;
    }
    return;
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
              rect = VRect_scale(rect, DEFAULT_PPM);
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

Object GameEntityProto = {
    .destroy = GameEntity_destroy,
    .init = GameEntity_init,
};

