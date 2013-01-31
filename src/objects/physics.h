#ifndef __physics_h
#define __physics_h
#include "../prefix.h"

#define DEFAULT_PPM 10

typedef struct PhysBox {
    float x;
    float y;
    float w;
    float h;
} PhysBox;

typedef struct Physics {
    Object proto;
} Physics;

int Physics_init(void *self);

extern Object PhysicsProto;

#endif
