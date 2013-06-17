#include "easer.h"
#include "engine.h"

float Easer_curve_linear(float progress) {
    return progress;
}

Easer *Easer_create(int length_ms, Easer_curve curve) {
    Easer *easer = calloc(1, sizeof(Easer));
    check(easer != NULL, "Couldn't create Easer");

    easer->length_ms = length_ms;
    easer->time_scale = 1.0;
    if (curve) {
        easer->curve = curve;
    } else {
        easer->curve = Easer_curve_linear;
    }

    return easer;
error:
    return NULL;
}

void Easer_destroy(Easer *easer) {
    check(easer != NULL, "No Easer to destroy");

    free(easer);

    return;
error:
    return;
}

void Easer_update(Easer *easer, struct Engine *engine, int delta_t) {
    easer->accumulator += delta_t * easer->time_scale;
    easer->accumulator = MIN(easer->length_ms, easer->accumulator);

    float progress = (float)easer->accumulator / (float)easer->length_ms;
    easer->value = easer->curve(progress);

    Scripting_call_hook(engine->scripting, easer, "update");
    if (easer->accumulator >= easer->length_ms) {
        easer->finished = 1;
        Scripting_call_hook(engine->scripting, easer, "finish");
    }
}
