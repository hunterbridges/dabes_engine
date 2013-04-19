#include <float.h>
#include <math.h>
#include "physics.h"

int Physics_init(void *self) {
    check_mem(self);
    Physics *physics = self;
    physics->max_dt = 16;
    physics->accumulator = 0;
    return 1;
error:
    return 0;
}

Object PhysicsProto = {
    .init = Physics_init
};

