#ifndef __sensor_h
#define __sensor_h
#include <chipmunk/chipmunk.h>
#include <lcthw/list.h>
#include "body.h"
#include "../prefix.h"
#include "../math/vpoint.h"

typedef struct Sensor {
    Body *body;

    float w;
    float h;
    VPoint offset;

    cpSpace *cp_space;
    cpShape *cp_shape;

    int on_static;
    List *on_sensors;
} Sensor;

Sensor *Sensor_create(float w, float h, VPoint offset);
void Sensor_destroy();
void Sensor_overlap_sensor(Sensor *a, Sensor *b);
void Sensor_separate_sensor(Sensor *a, Sensor *b);

#endif
