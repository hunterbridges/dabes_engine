#ifndef __physics_h
#define __physics_h
#include "../prefix.h"
#include "../core/stepper.h"
#include "../math/vpoint.h"

typedef struct Physics {
    Stepper *stepper;
} Physics;

Physics *Physics_create();
void Physics_destroy(Physics *physics);

#endif
