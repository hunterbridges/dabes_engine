#include "stepper.h"

Stepper *Stepper_create() {
    Stepper *stepper = calloc(1, sizeof(Stepper));
    check(stepper != NULL, "Couldn't create Stepper");
    return stepper;
error:
    return NULL;
}

void Stepper_destroy(Stepper *stepper) {
    check(stepper != NULL, "No Stepper to destroy");
    free(stepper);
    return;
error:
    return;
}

void Stepper_set_steps_per_second(Stepper *stepper,
        unsigned int steps_per_second) {
    stepper->steps_per_second = steps_per_second;

    if (steps_per_second > 0)
        stepper->step_skip = 1000.0 / steps_per_second;
    else
        stepper->step_skip = 0;
}

int Stepper_update(Stepper *stepper, long unsigned int step_ticks) {
    if(stepper->step_skip == 0) return 0;

    int accumulation = step_ticks % stepper->step_skip;
    int pure_steps = step_ticks / stepper->step_skip;
    stepper->stack += pure_steps;

    stepper->accumulator += accumulation;
    int accumulated_steps = stepper->accumulator / stepper->step_skip;
    stepper->stack += accumulated_steps;
    stepper->accumulator %= stepper->step_skip;
    return pure_steps + accumulated_steps;
}

int Stepper_reset(Stepper *stepper) {
    stepper->stack = 0;
    stepper->accumulator = 0;
    return 1;
}

int Stepper_pop(Stepper *stepper) {
    if (stepper->stack == 0) return 0;
    stepper->stack--;
    return 1;
}

int Stepper_clear(Stepper *stepper) {
    int stack = stepper->stack;
    stepper->stack = 0;
    return stack;
}
