#include "sensor.h"

Sensor *Sensor_create(float w, float h, VPoint offset) {
    Sensor *sensor = calloc(1, sizeof(Sensor));
    check(sensor != NULL, "Couldn't create sensor");

    sensor->w = w;
    sensor->h = h;
    sensor->offset = offset;

    sensor->on_sensors = List_create();

    return sensor;
error:
    return NULL;
}

void Sensor_destroy(Sensor *sensor) {
    check(sensor != NULL, "No sensor to destroy");

    if (sensor->body) {
        Body_remove_sensor(sensor->body, sensor);
    }

    free(sensor);
    return;
error:
    return;
}

void Sensor_overlap_sensor(Sensor *a, Sensor *b) {
    List_push(a->on_sensors, b);
}

void Sensor_separate_sensor(Sensor *a, Sensor *b) {
    List_remove_value(a->on_sensors, b);
}
