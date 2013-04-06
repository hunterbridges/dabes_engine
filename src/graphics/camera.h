#ifndef __camera_h
#define __camera_h
#include "graphics.h"

typedef struct Camera {
    struct VPoint focal;
    struct GfxSize screen_size;
    struct GfxSize scene_size;
    void *track_entity;

    VRectInset margin;
    double scale;
    double rotation_radians;
    struct VPoint translation;
} Camera;

Camera *Camera_create(int width, int height);
void Camera_track(Camera *camera);
void Camera_destroy(Camera *camera);
void Graphics_project_camera(Graphics *graphics, Camera *camera);
VRect Camera_screen_rect(Camera *camera);
VRect Camera_tracking_rect(Camera *camera);
VPoint Camera_project_point(Camera *camera, VPoint point);
VRect Camera_project_rect(Camera *camera, VRect rect);

#endif
