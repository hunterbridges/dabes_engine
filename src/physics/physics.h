#ifndef __physics_h
#define __physics_h
#include "../prefix.h"
#include "../math/vpoint.h"

typedef struct Physics {
    Object proto;
    long unsigned int max_dt;
    long unsigned int accumulator;
} Physics;

int Physics_init(void *self);

extern Object PhysicsProto;

#endif
