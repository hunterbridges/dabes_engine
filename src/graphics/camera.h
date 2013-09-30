#ifndef __camera_h
#define __camera_h
#include "../math/vpoint.h"
#include "graphics.h"

struct Entity;
typedef struct Camera {
    struct GfxSize screen_size;

    int has_scene_size;
    struct GfxSize scene_size;

    int num_entities;
    struct Entity **track_entities;

    int snap_to_scene;
    double max_scale;
    double min_scale;

    double lerp;

    VRectInset margin;
    VPoint focal;
    double scale;
    double rotation_radians;
    VPoint translation;

    struct {
        VPoint focal;
        double scale;
        double rotation_radians;
        VPoint translation;
    } tracking;
} Camera;

Camera *Camera_create(int width, int height);
void Camera_set_scene_size(Camera *camera, GfxSize scene_size);
void Camera_track_entities(Camera *camera, int num_entities,
        struct Entity **entities);
void Camera_track(Camera *camera);
void Camera_snap_tracking(Camera *camera);
void Camera_destroy(Camera *camera);
void Graphics_project_camera(Graphics *graphics, Camera *camera);
void Graphics_project_screen_camera(Graphics *graphics, Camera *camera);
VRect Camera_base_rect(Camera *camera);
VRect Camera_visible_rect(Camera *camera, int lerped);
VRect Camera_tracking_rect(Camera *camera);
VRect Camera_project_rect(Camera *camera, VRect rect, int apply_translation,
                          int lerped);
void Camera_debug(Camera *camera, Graphics *graphics);

// World point -> screen point
VPoint Camera_project_point(Camera *camera, VPoint point, int apply_translation,
                            int lerped);

// Screen point -> world point
VPoint Camera_cast_point(Camera *camera, VPoint point, int lerped);

#endif
