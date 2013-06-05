#include "chipmunk_body.h"

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
    float sh = snip * h2;

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

cpVect ccp(double x, double y) {
    cpVect cp = {x, y};
    return cp;
}

cpVect tocp(VPoint vp) {
    cpVect cp = {vp.x, vp.y};
    return cp;
}

VPoint tovp(cpVect cp) {
    VPoint vp = {.x = cp.x, .y = cp.y};
    return vp;
}

int ChipmunkBody_init(Body *body, float w, float h, float mass,
                      int can_rotate) {
    float moment = (can_rotate ? cpMomentForBox(mass, w, h) : INFINITY);
    cpBody *cp_body = cpBodyNew(mass, moment);
    body->cp_body = cp_body;
    cpBodySetUserData(cp_body, &body->state);

    cpShape *shape = cpBoxShapeNew(cp_body, w, h);
    cpShapeSetBody(shape, cp_body);
    body->cp_shape = shape;

    return 1;
}

void ChipmunkBody_cleanup(Body *body) {
    if (body->cp_space) {
        cpSpaceRemoveShape(body->cp_space, body->cp_shape);
        cpSpaceRemoveBody(body->cp_space, body->cp_body);
    }

    cpShapeDestroy(body->cp_shape);
    cpBodyDestroy(body->cp_body);
}

VRect ChipmunkBody_gfx_rect(Body *body, float pixels_per_meter, int rotate) {
    cpShape *shape = body->cp_shape;
    float rads = cpBodyGetAngle(shape->body);
    cpVect pos = cpBodyGetPos(shape->body);
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
    VPoint center = {pos.x, pos.y};
    center = VPoint_add(center, body->draw_offset);
    return VPoint_scale(center, pixels_per_meter);
}

void ChipmunkBody_apply_force(Body *body, VPoint force, VPoint offset) {
    cpBodyApplyForce(body->cp_body, tocp(force), tocp(offset));
}

void ChipmunkBody_set_hit_box(Body *body, float w, float h, VPoint offset) {
    if (body->cp_space) {
        if (body->cp_shape) {
            cpSpaceRemoveShape(body->cp_space, body->cp_shape);
            cpShapeDestroy(body->cp_shape);
        }
    }

    cpBody *cp_body = body->cp_body;

    VPoint flipped = VPoint_scale(offset, -1);
    body->draw_offset = flipped;

    cpShape *shape = cpSnipBoxShapeNew(cp_body, w * body->w, h * body->h, 0.25, ccp(0,0));
    cpShapeSetBody(shape, cp_body);
    body->cp_shape = shape;

    if (body->cp_space) {
        cpSpaceAddShape(body->cp_space, body->cp_shape);
    }
}

VPoint ChipmunkBody_get_pos(Body *body) {
    return tovp(cpBodyGetPos(body->cp_body));
}

void ChipmunkBody_set_pos(Body *body, VPoint pos) {
    return cpBodySetPos(body->cp_body, tocp(pos));
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
}

BodyProto ChipmunkBodyProto = {
    .init = ChipmunkBody_init,
    .cleanup = ChipmunkBody_cleanup,

    .gfx_rect = ChipmunkBody_gfx_rect,
    .gfx_center = ChipmunkBody_gfx_center,

    .apply_force = ChipmunkBody_apply_force,
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
    .get_mass = ChipmunkBody_get_mass,
    .set_mass = ChipmunkBody_set_mass,
    .get_can_rotate = ChipmunkBody_get_can_rotate,
    .set_can_rotate = ChipmunkBody_set_can_rotate
};
