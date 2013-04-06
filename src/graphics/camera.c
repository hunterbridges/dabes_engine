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
    VRectInset margin = {100, 100, 100, 100};
    camera->margin = margin;

    return camera;
error:
    return NULL;
}

void Camera_track(Camera *camera) {
    if (camera->track_entity) {
        GameEntity *entity = camera->track_entity;
        VRect t_rect = Camera_tracking_rect(camera);
        VRect e_rect = GameEntity_real_rect(entity);
        VRect t_bound = VRect_bounding_box(t_rect);
        VRect e_bound = Camera_project_rect(camera, e_rect);
        e_bound = VRect_bounding_box(e_bound);
        if (VRect_contains_rect(t_bound, e_bound)) return;
      
        int closest = 0;
        double min_mag = FLT_MAX;
        VPoint t_closest, e_closest;
        int i = 0;
        for (i = 0; i < 4; i++) {
          VPoint tv = VRect_vertex(t_bound, i);
          VPoint ev = VRect_vertex(e_bound, i);
          VPoint diff = VPoint_subtract(ev, tv);
          double mag = VPoint_magnitude(diff);
          if (mag < min_mag) {
              t_closest = tv;
              e_closest = ev;
              min_mag = mag;
              closest = i;
          }
        }
      
        VPoint diff = {0,0};
        VPointRel close_rel = VPoint_rel(e_closest, t_closest);
        if ((closest == 0 || closest == 1) &&
               (close_rel & VPointRelYLess)) {
            diff.y += e_closest.y - t_closest.y;
        }
        if ((closest == 2 || closest == 3) &&
               (close_rel & VPointRelYMore)) {
            diff.y += e_closest.y - t_closest.y;
        }
      
        if ((closest == 0 || closest == 3) &&
               (close_rel & VPointRelXLess)) {
            diff.x += e_closest.x - t_closest.x;
        }
        if ((closest == 1 || closest == 2) &&
               (close_rel & VPointRelXMore)) {
            diff.x = e_closest.x - t_closest.x;
        }
        camera->focal = VPoint_add(camera->focal,
                                   VPoint_scale(diff, 1 / (2 * camera->scale)));
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

VRect Camera_base_rect(Camera *camera) {
  VPoint delta = {
    camera->screen_size.w / 2.0,
    camera->screen_size.h / 2.0
  };
  
  VRect cam_rect = VRect_from_xywh(-delta.x, -delta.y,
                                   camera->screen_size.w,
                                   camera->screen_size.h);
  return cam_rect;
}

VRect Camera_visible_rect(Camera *camera) {
  VRect cam_rect = Camera_base_rect(camera);
  cam_rect = VRect_scale(cam_rect, 1 / camera->scale);
  cam_rect = VRect_rotate(cam_rect, VRect_center(cam_rect),
                          camera->rotation_radians);
  cam_rect = VRect_move(cam_rect, camera->translation);
  return cam_rect;
}

VRect Camera_screen_rect(Camera *camera) {
  
};

VRect Camera_tracking_rect(Camera *camera) {
  VRect new = VRect_inset(Camera_base_rect(camera), camera->margin);
  return new;
}

VPoint Camera_project_point(Camera *camera, VPoint point) {
  VPoint cam_center = camera->focal;
  VPoint new = VPoint_subtract(point, cam_center);
  
  new = VPoint_rotate(new, VPointZero, camera->rotation_radians);
  new = VPoint_scale(new, camera->scale);
  new = VPoint_subtract(new, camera->translation);
  
  return new;
}

VRect Camera_project_rect(Camera *camera, VRect rect) {
  VRect new = rect;
  new.tl = Camera_project_point(camera, rect.tl);
  new.tr = Camera_project_point(camera, rect.tr);
  new.br = Camera_project_point(camera, rect.br);
  new.bl = Camera_project_point(camera, rect.bl);
  return new;
}