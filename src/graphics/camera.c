#include "camera.h"
#include "game_entity.h"

// CAMERA
Camera *Camera_create(int width, int height) {
    Camera *camera = calloc(1, sizeof(Camera));
    check(camera != NULL, "No camera created");

    camera->screen_size.w = width;
    camera->screen_size.h = height;
    camera->focal.x = width / 2.0;
    camera->focal.y = height / 2.0;
    camera->scale = 1;
    camera->rotation_radians = 0;
    camera->translation.x = 0;
    camera->translation.y = 0;

    return camera;
error:
    return NULL;
}

void Camera_track(Camera *camera) {
    if (camera->track_entity) {
        GameEntity *entity = camera->track_entity;
        camera->focal = GameEntity_center(entity);
    } else {
        camera->focal.x += camera->translation.x;
        camera->focal.y += camera->translation.y;
        camera->translation.x = 0;
        camera->translation.y = 0;
    }
}

void Camera_destroy(Camera *camera) {
    free(camera);
}

void Graphics_project_camera(Graphics *graphics, Camera *camera) {
    Graphics_reset_projection_matrix(graphics);

    // We think of our game space with <0, 0> as top left corner
    // GL treats <0, 0> as center of viewport
    Graphics_ortho_projection_matrix(graphics,
            -camera->screen_size.w / 2.0,
            camera->screen_size.w / 2.0,
            -camera->screen_size.h / 2.0,
            camera->screen_size.h / 2.0,
            1.0, -1.0);
    Graphics_scale_projection_matrix(graphics, camera->scale,
            camera->scale, 1);
    Graphics_rotate_projection_matrix(graphics, camera->rotation_radians,
            0, 0, -1);
    Graphics_translate_projection_matrix(graphics,
                                         -camera->focal.x,
                                         -camera->focal.y,
                                         0);
}

