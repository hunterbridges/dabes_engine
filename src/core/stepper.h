#ifndef __stepper_h
#define __stepper_h
#include "../prefix.h"

typedef struct Stepper {
    unsigned int steps_per_second;
    unsigned int stack;

    long unsigned int step_skip;
    unsigned int accumulator;
} Stepper;

Stepper *Stepper_create();
void Stepper_destroy(Stepper *stepper);
void Stepper_set_steps_per_second(Stepper *stepper,
        unsigned int steps_per_second);
int Stepper_update(Stepper *stepper, long unsigned int step_ticks);
int Stepper_reset(Stepper *stepper);
int Stepper_pop(Stepper *stepper);
int Stepper_clear(Stepper *stepper);

#endif
