#include "body.h"

Body *Body_create(BodyProto proto, float w, float h, float mass,
        int can_rotate) {
    Body *body = calloc(1, sizeof(Body));
    check(body != NULL, "Couldn't create Body");

    body->proto = proto;

    body->can_rotate = can_rotate;
    body->mass = mass;
    body->w = w;
    body->h = h;

    int ret = body->_(init)(body, w, h, mass, can_rotate);
    check(ret, "Couldn't configure Body")

    return body;
error:
    if (body) {
        free(body);
    }
    return NULL;
}

void Body_destroy(Body *body) {
    check(body != NULL, "No Body to destroy");
    body->_(cleanup)(body);
    free(body);
    return;
error:
    return;
}
