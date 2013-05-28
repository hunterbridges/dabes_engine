#include "chipmunk_body.h"

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
    cpBody *fixture = cpBodyNew(mass, moment);

    cpShape *shape = cpBoxShapeNew(fixture, w, h);
    cpBodySetUserData(fixture, &body->state);

    body->cp_shape = shape;
    body->cp_body = fixture;
    return 1;
}

void ChipmunkBody_cleanup(Body *body) {
    if (body->cp_space) {
        cpSpaceRemoveShape(body->cp_space, body->cp_shape);
        cpSpaceRemoveBody(body->cp_space, body->cp_body);
    }

    cpShapeDestroy(body->cp_shape);
    cpBodyDestroy(body->cp_body);
    free(body);
}

void ChipmunkBody_apply_force(Body *body, VPoint force, VPoint offset) {
    cpBodyApplyForce(body->cp_body, tocp(force), tocp(offset));
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
    return cpShapeGetFriction(body->cp_shape);
}

void ChipmunkBody_set_friction(Body *body, float friction) {
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

    .apply_force = ChipmunkBody_apply_force,

    .get_pos = ChipmunkBody_get_pos,
    .set_pos = ChipmunkBody_set_pos,
    .get_velo = ChipmunkBody_get_velo,
    .set_velo = ChipmunkBody_set_velo,
    .get_force = ChipmunkBody_get_force,
    .set_force = ChipmunkBody_set_force,
    .get_angle = ChipmunkBody_get_angle,
    .set_angle = ChipmunkBody_set_angle,
    .get_friction = ChipmunkBody_get_friction,
    .set_friction = ChipmunkBody_set_friction
};
