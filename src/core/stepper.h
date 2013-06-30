#ifndef __stepper_h
#define __stepper_h
#include "../prefix.h"

typedef struct Stepper {
    float steps_per_second;
    unsigned long stack;

    unsigned long step_skip;
    unsigned long accumulator;
} Stepper;

Stepper *Stepper_create();
void Stepper_destroy(Stepper *stepper);
void Stepper_set_steps_per_second(Stepper *stepper, float steps_per_second);
unsigned long Stepper_update(Stepper *stepper, long unsigned int step_ticks);
int Stepper_reset(Stepper *stepper);
int Stepper_pop(Stepper *stepper);
unsigned long Stepper_clear(Stepper *stepper);

#endif
