#include <chipmunk/chipmunk.h>
#include "../scenes/chipmunk_scene.h"
#include "chipmunk_body.h"
#include "sensor.h"

typedef struct GroundingContext {
	cpVect normal;
	cpVect impulse;
	cpFloat penetration;

	cpBody *body;
} GroundingContext;

void GroundingContext_callback(cpBody *UNUSED(body), cpArbiter *arb, GroundingContext *grounding){
	CP_ARBITER_GET_BODIES(arb, b1, b2);
	cpVect n = cpvneg(cpArbiterGetNormal(arb, 0));

	if(n.y < grounding->normal.y){
		grounding->normal = n;
		grounding->penetration = -cpArbiterGetDepth(arb, 0);
		grounding->body = b2;
	}

	grounding->impulse = cpvadd(grounding->impulse, cpArbiterTotalImpulseWithFriction(arb));
}

void GroundingContext_update(GroundingContext *context, cpBody *body)
{
	(*context) = (GroundingContext){cpvzero, cpvzero, 0.0, NULL};
	cpBodyEachArbiter(body, (cpBodyArbiterIteratorFunc)GroundingContext_callback,
                    context);
}

typedef struct ChipmunkBodyContext {
    GroundingContext grounding;
} ChipmunkBodyContext;

void ChipmunkBody_update_velocity(cpBody *body, cpVect gravity, cpFloat damping, cpFloat dt)
{
    Body *dab_body = cpBodyGetUserData(body);
    ChipmunkBodyContext *context = (ChipmunkBodyContext *)dab_body->context;
    GroundingContext_update(&context->grounding, body);

    // TODO: Fancier stuff
    cpBodyUpdateVelocity(body, gravity, damping, dt);
}

void ChipmunkBody_set_hit_box(Body *body, float w, float h, VPoint offset);

cpShape *cpOffsetBoxShapeNew(cpBody *body, float w, float h, cpVect offset) {
    float w2 = w / 2.0;
    float h2 = h / 2.0;

    cpVect verts[4] = {
        {-w2, -h2},
        {-w2,  h2},
        { w2,  h2},
        { w2, -h2},
    };

    cpShape *shape = cpPolyShapeNew(body, 4, verts, offset);

    return shape;
}

cpShape *cpSnipBoxShapeNew(cpBody *body, float w, float h, float snip,
        cpVect offset) {
    float w2 = w / 2.0;
    float h2 = h / 2.0;
    float sw = snip * w2;
    float sh = snip * w2;

    cpVect verts[8] = {
        { w2 - sw, -h2},
        {-w2 + sw, -h2},      // Top Edge
        {-w2,      -h2 + sh},
        {-w2,       h2 - sh}, // Left Edge
        {-w2 + sw,  h2},
        { w2 - sw,  h2},      // Bottom Edge
        { w2,       h2 - sh},
        { w2,      -h2 + sh}, // Right Edge
    };

    cpShape *shape = cpPolyShapeNew(body, 8, verts, offset);

    return shape;
}

cpShape *cpRoundBoxShapeNew(cpBody *body, float w, float h, float snip,
        cpVect offset) {
    float w2 = w / 2.0;
    float h2 = h / 2.0;
    float sw = snip * w2;
    float sh = snip * h2;
    float s2 = snip / 2.0;
    float sw2 = s2 * w2;
    float sh2 = s2 * h2;

    cpVect verts[12] = {
        { w2 - sw,   -h2},
        {-w2 + sw,   -h2},       // Top Edge
        {-w2 + sw2,  -h2 + sh2},
        {-w2,        -h2 + sh},
        {-w2,         h2 - sh},  // Left Edge
        {-w2 + sw2,   h2 - sh2},
        {-w2 + sw,    h2},
        { w2 - sw,    h2},       // Bottom Edge
        { w2 - sw2,   h2 - sh2},
        { w2,         h2 - sh},
        { w2,        -h2 + sh},  // Right Edge
        { w2 - sw2,  -h2 + sh2}
    };

    cpShape *shape = cpPolyShapeNew(body, 12, verts, offset);

    return shape;
}

static inline cpVect ccp(double x, double y) {
    cpVect cp = {x, y};
    return cp;
}

static inline cpVect tocp(VPoint vp) {
    cpVect cp = {vp.x, vp.y};
    return cp;
}

static inline VPoint tovp(cpVect cp) {
    VPoint vp = {.x = cp.x, .y = cp.y};
    return vp;
}

int ChipmunkBody_init(Body *body, float w, float h, float mass,
                      int can_rotate) {
    float moment = (can_rotate ? cpMomentForBox(mass, w, h) : INFINITY);
    cpBody *cp_body = cpBodyNew(mass, moment);
    body->cp_body = cp_body;
    body->context = calloc(1, sizeof(ChipmunkBodyContext));
    cpBodySetUserData(cp_body, body);
    cp_body->velocity_func = ChipmunkBody_update_velocity;

    cpShape *shape = cpBoxShapeNew(cp_body, w, h);
    cpShapeSetCollisionType(shape, OCSCollisionTypeEntity);
    body->cp_shape = shape;
  
    body->collision_layers = CP_ALL_LAYERS;

    return 1;
}

void body_clear_shapes(cpBody *cpBody, cpShape *shape, void *data) {
    Body *body = data;
    if (shape->space_private) {
      cpSpaceRemoveShape(shape->space_private, shape);
    }
    cpShapeSetBody(shape, NULL);
    cpShapeFree(shape);
    if (shape == body->cp_shape) body->cp_shape = NULL;
}

void ChipmunkBody_cleanup(Body *body) {
    if (!body->cp_body) return;

    free(body->context);

    cpBodyEachShape(body->cp_body, body_clear_shapes, body);

    if (body->cp_shape) {
        cpShapeSetBody(body->cp_shape, NULL);
        cpShapeFree(body->cp_shape);
        body->cp_shape= NULL;
    }

    if (cpBodyGetSpace(body->cp_body)) {
        if (!body->is_rogue) {
            cpSpaceRemoveBody(body->cp_space, body->cp_body);
            body->cp_body = NULL;
        }
    }

    cpBodyFree(body->cp_body);
}

VRect ChipmunkBody_gfx_rect(Body *body, float pixels_per_meter, int rotate) {
    cpShape *shape = body->cp_shape;
    float rads = cpBodyGetAngle(shape->body);
    cpVect pos = cpBodyGetPos(shape->body);
    ChipmunkBodyContext *ctx = (ChipmunkBodyContext *)body->context;
    pos = cpvadd(pos, cpvmult(ctx->grounding.normal, ctx->grounding.penetration));
    VPoint center = {pos.x, pos.y};
    center = VPoint_add(center, body->draw_offset);
    VRect rect = VRect_from_xywh(center.x - body->w / 2, center.y - body->h / 2,
                                 body->w, body->h);
    if (rotate) rect = VRect_rotate(rect, center, rads);
    rect = VRect_scale(rect, pixels_per_meter);
    return rect;
}

VPoint ChipmunkBody_gfx_center(Body *body, float pixels_per_meter) {
    cpShape *shape = body->cp_shape;
    cpVect pos = cpBodyGetPos(shape->body);
    ChipmunkBodyContext *ctx = (ChipmunkBodyContext *)body->context;
    pos = cpvadd(pos, cpvmult(ctx->grounding.normal, ctx->grounding.penetration));
    VPoint center = {pos.x, pos.y};
    center = VPoint_add(center, body->draw_offset);
    return VPoint_scale(center, pixels_per_meter);
}

void ChipmunkBody_add_sensor(Body *body, Sensor *sensor) {
    cpVect cp_offset = {sensor->offset.x, sensor->offset.y};
    cpShape *shape =
        cpOffsetBoxShapeNew(body->cp_body, sensor->w, sensor->h, cp_offset);
    cpShapeSetBody(shape, body->cp_body);
    cpShapeSetSensor(shape, 1);
    cpShapeSetUserData(shape, sensor);
    cpShapeSetCollisionType(shape, OCSCollisionTypeSensor);
    cpShapeSetLayers(shape, body->collision_layers);
    sensor->body = body;
    sensor->cp_shape = shape;

    if (body->cp_space) {
        cpSpaceAddShape(body->cp_space, shape);
        sensor->cp_space = body->cp_space;
    }
}

void ChipmunkBody_remove_sensor(Body *body, Sensor *sensor) {
    if (cpShapeGetSpace(sensor->cp_shape)) {
        cpSpaceRemoveShape(sensor->cp_space, sensor->cp_shape);
    }

    cpShapeSetBody(sensor->cp_shape, NULL);
    cpShapeFree(sensor->cp_shape);
    sensor->cp_shape = NULL;
    sensor->body = NULL;
    sensor->cp_space = NULL;
}

void ChipmunkBody_apply_force(Body *body, VPoint force, VPoint offset) {
    cpBodyApplyForce(body->cp_body, tocp(force), tocp(offset));
}

void ChipmunkBody_apply_impulse(Body *body, VPoint impulse, VPoint offset) {
    cpBodyApplyImpulse(body->cp_body, tocp(impulse), tocp(offset));
}

void ChipmunkBody_set_hit_box(Body *body, float w, float h, VPoint offset) {
    if (body->cp_space) {
        if (body->cp_shape) {
            cpSpaceRemoveShape(body->cp_space, body->cp_shape);
        }
    }

    if (body->cp_shape) {
        cpShapeFree(body->cp_shape);
    }

    cpBody *cp_body = body->cp_body;

    VPoint flipped = VPoint_scale(offset, -1);
    body->draw_offset = flipped;

    cpShape *shape = cpSnipBoxShapeNew(cp_body, w * body->w, h * body->h, 0.25, ccp(0,0));
    cpShapeSetBody(shape, cp_body);
    body->cp_shape = shape;

    if (body->is_static) {
        cpShapeSetSensor(shape, 1);
        cpShapeSetCollisionType(body->cp_shape, OCSCollisionTypeStaticEntity);
    } else {
        cpShapeSetCollisionType(body->cp_shape, OCSCollisionTypeEntity);
    }

    if (body->cp_space) {
        cpSpaceAddShape(body->cp_space, body->cp_shape);
    }
  
    cpShapeSetLayers(shape, body->collision_layers);

    GfxSize sz = {.w = w, .h = h};
    if (body->state.entity) {
      body->state.entity->size = sz;
    }
}

VPoint ChipmunkBody_get_pos(Body *body) {
    return tovp(cpBodyGetPos(body->cp_body));
}

void ChipmunkBody_set_pos(Body *body, VPoint pos) {
    cpBodySetPos(body->cp_body, tocp(pos));
    if (body->state.entity) {
      body->state.entity->center = pos;
    }
}

VPoint ChipmunkBody_get_velo(Body *body) {
    return tovp(cpBodyGetVel(body->cp_body));
}

void ChipmunkBody_set_velo(Body *body, VPoint velo) {
    return cpBodySetVel(body->cp_body, tocp(velo));
}

VPoint ChipmunkBody_get_force(Body *body) {
    return tovp(cpBodyGetForce(body->cp_body));
}

void ChipmunkBody_set_force(Body *body, VPoint force) {
    return cpBodySetForce(body->cp_body, tocp(force));
}

float ChipmunkBody_get_angle(Body *body) {
    return cpBodyGetAngle(body->cp_body);
}

void ChipmunkBody_set_angle(Body *body, float angle) {
    cpBodySetAngle(body->cp_body, angle);
}

float ChipmunkBody_get_friction(Body *body) {
    if (body->cp_shape)
        return cpShapeGetFriction(body->cp_shape);
    return 0;
}

void ChipmunkBody_set_friction(Body *body, float friction) {
    if (body->cp_shape)
        cpShapeSetFriction(body->cp_shape, friction);
}

float ChipmunkBody_get_elasticity(Body *body) {
    if (body->cp_shape)
        return cpShapeGetElasticity(body->cp_shape);
    return 0;
}

void ChipmunkBody_set_elasticity(Body *body, float elasticity) {
    if (body->cp_shape)
        cpShapeSetElasticity(body->cp_shape, elasticity);
}

float ChipmunkBody_get_mass(Body *body) {
    return cpBodyGetMass(body->cp_body);
}

void ChipmunkBody_set_mass(Body *body, float mass) {
    body->mass = mass;
    float moment = (body->can_rotate ?
                    cpMomentForBox(mass, body->w, body->h) :
                    INFINITY);
    cpBodySetMass(body->cp_body, mass);
    cpBodySetMoment(body->cp_body, moment);
}

int ChipmunkBody_get_can_rotate(Body *body) {
    return body->can_rotate;
}

void ChipmunkBody_set_can_rotate(Body *body, int can_rotate) {
    body->can_rotate = can_rotate;
    float moment = (can_rotate ?
                    cpMomentForBox(body->mass, body->w, body->h) :
                    INFINITY);
    cpBodySetMoment(body->cp_body, moment);
    cpBodySetAngVel(body->cp_body, 0);
    cpBodySetTorque(body->cp_body, 0);
}

int ChipmunkBody_get_is_rogue(Body *body) {
    return body->is_rogue;
}

void ChipmunkBody_set_is_rogue(Body *body, int is_rogue) {
    if (is_rogue == body->is_rogue) return;
    body->is_rogue = is_rogue;

    if (is_rogue) {
        cpBodySetMass(body->cp_body, INFINITY);
        cpBodySetMoment(body->cp_body, INFINITY);

        if (body->cp_space)
            cpSpaceRemoveBody(body->cp_space, body->cp_body);
    } else {
        ChipmunkBody_set_mass(body, body->mass);
        if (body->cp_space)
            cpSpaceAddBody(body->cp_space, body->cp_body);
    }
}

int ChipmunkBody_get_is_static(Body *body) {
    return body->is_static;
}

void ChipmunkBody_set_is_static(Body *body, int is_static) {
    body->is_static = is_static;

    if (is_static) {
        ChipmunkBody_set_is_rogue(body, 1);
        cpShapeSetSensor(body->cp_shape, 1);
        cpShapeSetCollisionType(body->cp_shape, OCSCollisionTypeStaticEntity);
    } else {
        cpShapeSetSensor(body->cp_shape, 0);
        cpShapeSetCollisionType(body->cp_shape, OCSCollisionTypeEntity);
    }
}

int ChipmunkBody_get_collision_layers(Body *body) {
    return cpShapeGetLayers(body->cp_shape);
}

void ChipmunkBody_set_collision_layers(Body *body, dab_uint32 collision_layers) {
    body->collision_layers = collision_layers;
    cpShapeSetLayers(body->cp_shape, collision_layers);
    LIST_FOREACH(body->sensors, first, next, current) {
        Sensor *sensor = current->value;
        cpShapeSetLayers(sensor->cp_shape, collision_layers);
    }
}

BodyProto ChipmunkBodyProto = {
    .init = ChipmunkBody_init,
    .cleanup = ChipmunkBody_cleanup,

    .gfx_rect = ChipmunkBody_gfx_rect,
    .gfx_center = ChipmunkBody_gfx_center,

    .add_sensor = ChipmunkBody_add_sensor,
    .remove_sensor = ChipmunkBody_remove_sensor,

    .apply_force = ChipmunkBody_apply_force,
    .apply_impulse = ChipmunkBody_apply_impulse,
    .set_hit_box = ChipmunkBody_set_hit_box,

    .get_pos = ChipmunkBody_get_pos,
    .set_pos = ChipmunkBody_set_pos,
    .get_velo = ChipmunkBody_get_velo,
    .set_velo = ChipmunkBody_set_velo,
    .get_force = ChipmunkBody_get_force,
    .set_force = ChipmunkBody_set_force,
    .get_angle = ChipmunkBody_get_angle,
    .set_angle = ChipmunkBody_set_angle,
    .get_friction = ChipmunkBody_get_friction,
    .set_friction = ChipmunkBody_set_friction,
    .get_elasticity = ChipmunkBody_get_elasticity,
    .set_elasticity = ChipmunkBody_set_elasticity,
    .get_mass = ChipmunkBody_get_mass,
    .set_mass = ChipmunkBody_set_mass,
    .get_can_rotate = ChipmunkBody_get_can_rotate,
    .set_can_rotate = ChipmunkBody_set_can_rotate,
    .get_is_rogue = ChipmunkBody_get_is_rogue,
    .set_is_rogue = ChipmunkBody_set_is_rogue,
    .get_is_static = ChipmunkBody_get_is_static,
    .set_is_static = ChipmunkBody_set_is_static,
    .get_collision_layers = ChipmunkBody_get_collision_layers,
    .set_collision_layers = ChipmunkBody_set_collision_layers
};

