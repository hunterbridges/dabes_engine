#ifndef __camera_h
#define __camera_h
#include "graphics.h"

typedef struct Camera {
    struct GfxPoint focal;
    struct GfxSize screen_size;
    void *track_entity;

    double scale;
    double rotation_radians;
    struct GfxPoint translation;
} Camera;

Camera *Camera_create(int width, int height);
void Camera_track(Camera *camera);
void Camera_destroy(Camera *camera);
void Graphics_project_camera(Graphics *graphics, Camera *camera);

#endif
