#ifndef __easer_h
#define __easer_h
#include "../prefix.h"

typedef float (*Easer_curve)(float progress);

struct Engine;
typedef struct Easer {
    int length_ms;
    int accumulator;
    int finished;
    float time_scale;
    float value;
    Easer_curve curve;
} Easer;

float Easer_curve_linear(float progress);

Easer *Easer_create(int length_ms, Easer_curve curve);
void Easer_destroy(Easer *easer);
void Easer_update(Easer *easer, struct Engine *engine, unsigned long delta_t);

#endif
