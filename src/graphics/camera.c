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

void Camera_correct(Camera *camera) {
    if (!camera->snap_to_scene) return;
    GfxSize raw_scene = camera->scene_size;
    GfxSize proj_scene = {
        camera->scene_size.w * camera->scale,
        camera->scene_size.h * camera->scale
    };
    VPoint scene_center = {
        raw_scene.w / 2.0,
        raw_scene.h / 2.0
    };
    if (proj_scene.w <= camera->screen_size.w) {
        camera->focal.x = scene_center.x;
    } else {
        camera->focal.x =
            MIN(camera->scene_size.w - camera->screen_size.w / (2.0 * camera->scale),
                MAX(camera->screen_size.w / (2.0 * camera->scale), camera->focal.x));
    }
    if (proj_scene.h <= camera->screen_size.h) {
        camera->focal.y = scene_center.y;
    } else {
        camera->focal.y =
            MIN(camera->scene_size.h - camera->screen_size.h / (2.0 * camera->scale),
                MAX(camera->screen_size.h / (2.0 * camera->scale), camera->focal.y));
    }
}

void Camera_track(Camera *camera) {
    if (camera->track_entity) {
        GameEntity *entity = camera->track_entity;
        VRect t_rect = Camera_tracking_rect(camera);
        VRect e_rect = GameEntity_real_rect(entity);
        VRect t_bound = VRect_bounding_box(t_rect);
        VRect e_bound = Camera_project_rect(camera, e_rect);
        e_bound = VRect_bounding_box(e_bound);
        if (VRect_contains_rect(t_bound, e_bound)) {
            Camera_correct(camera);
            return;
        }

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

        VPoint diff = {.x=0,.y=0};
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
        diff = VPoint_scale(diff, 1 / camera->scale);
        diff = VPoint_rotate(diff, VPointZero, camera->rotation_radians);
        camera->focal = VPoint_add(camera->focal, diff);

        VPoint e_center = VRect_center(e_rect);
        if (VRect_width(e_bound) >= VRect_width(t_bound))
            camera->focal.x = e_center.x;
        if (VRect_height(e_bound) >= VRect_height(t_bound))
            camera->focal.y = e_center.y;
    } else {
        camera->focal.x += camera->translation.x;
        camera->focal.y += camera->translation.y;
        camera->translation.x = 0;
        camera->translation.y = 0;
    }

    Camera_correct(camera);
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
    Graphics_rotate_projection_matrix(graphics,
                                      camera->rotation_radians * 180 / M_PI,
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
  cam_rect = VRect_move(cam_rect, camera->focal);
  return cam_rect;
}

VRect Camera_tracking_rect(Camera *camera) {
  VRect new = VRect_inset(Camera_base_rect(camera), camera->margin);
  return new;
}

VPoint Camera_project_point(Camera *camera, VPoint point) {
  VPoint cam_center = camera->focal;
  VPoint new = VPoint_subtract(point, cam_center);

  new = VPoint_rotate(new, VPointZero, -camera->rotation_radians);
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

void Camera_debug(Camera *camera, Graphics *graphics) {
    Camera screen_cam = {
      .focal = {0, 0},
      .screen_size = camera->screen_size,
      .scale = 1,
      .rotation_radians = 0,
      .margin = camera->margin
    };
    Graphics_reset_modelview_matrix(graphics);
    Graphics_reset_projection_matrix(graphics);
    Graphics_project_camera(graphics, &screen_cam);
    GLfloat cam_color[4] = {1, 0, 0, 1};
    VRect track_rect = Camera_tracking_rect(&screen_cam);
    Graphics_stroke_rect(graphics, track_rect, cam_color, 0, 0);

    if (camera->track_entity) {
        GLfloat e_color[4] = {0, 1, 0, 1};
        GameEntity *entity = camera->track_entity;
        VRect e_rect = GameEntity_real_rect(entity);
        VRect e_bound = Camera_project_rect(camera, e_rect);
        e_bound = VRect_bounding_box(e_bound);
        Graphics_stroke_rect(graphics, e_bound, e_color, 0, 0);
    }
}
