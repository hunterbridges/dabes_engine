#include <float.h>
#include <math.h>
#include "physics.h"

Physics *Physics_create() {
    Physics *physics = calloc(1, sizeof(Physics));
    check(physics != NULL, "Couldn't create Physics");

    physics->stepper = Stepper_create();
    Stepper_set_steps_per_second(physics->stepper, 60);

    return physics;
error:
    return NULL;
}

void Physics_destroy(Physics *physics) {
    check(physics != NULL, "No Physics to destroy");
    Stepper_destroy(physics->stepper);
    free(physics);
    return;
error:
    return;
}
