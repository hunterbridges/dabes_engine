#include "camera.h"
#include "../entities/entity.h"

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
    camera->num_entities = 0;
    camera->max_scale = -1;
    camera->min_scale = -1;
    camera->lerp = 1.0;
    VRectInset margin = {100, 100, 100, 100};
    camera->margin = margin;

    Camera_snap_tracking(camera);

    return camera;
error:
    return NULL;
}

void Camera_track_entities(Camera *camera, int num_entities,
        struct Entity **entities) {
    check(camera != NULL, "No camera provided");
    if (camera->track_entities) free(camera->track_entities);
    if (num_entities > 0) {
        camera->track_entities = calloc(num_entities, sizeof(Entity *));
        memcpy(camera->track_entities, entities, sizeof(Entity *) * num_entities);
    } else {
        camera->track_entities = NULL;
    }
    camera->num_entities = num_entities;
    return;
error:
    return;
}

void Camera_set_scene_size(Camera *camera, GfxSize scene_size) {
    camera->has_scene_size = 1;
    camera->scene_size = scene_size;
}

void Camera_correct_scale(Camera *camera) {
    if (camera->min_scale > 0 ) {
        camera->scale = MAX(camera->scale, camera->min_scale);
    }
    if (camera->max_scale > 0 ) {
        camera->scale = MIN(camera->scale, camera->max_scale);
    }
}

void Camera_correct_focal(Camera *camera) {
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

static void Camera_track_single(Camera *camera) {
    Entity *entity = *camera->track_entities;
    VRect t_rect = Camera_tracking_rect(camera);
    VRect e_rect = Entity_real_rect(entity);
    VRect t_bound = VRect_bounding_box(t_rect);
    VRect e_bound = Camera_project_rect(camera, e_rect, 0, 0);
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
}

static void Camera_track_multi(Camera *camera) {
    Entity **e = camera->track_entities;
    int i = 0;
    VPoint min = {FLT_MAX, FLT_MAX};
    VPoint max = {FLT_MIN, FLT_MIN};
    for (i = 0; i < camera->num_entities; i++, e++) {
        Entity *entity = *e;
        VRect e_rect = Entity_real_rect(entity);
        VRect e_bound = VRect_bounding_box(e_rect);
        min.x = MIN(min.x, e_bound.tl.x);
        min.y = MIN(min.y, e_bound.tl.y);
        max.x = MAX(max.x, e_bound.br.x);
        max.y = MAX(max.y, e_bound.br.y);
    }
    VPoint center = VPoint_mid(min, max);
    camera->focal = center;
    GfxSize track_size = {max.x - min.x, max.y - min.y};
    GfxSize screen_size = {camera->screen_size.w - camera->margin.left - camera->margin.right,
                           camera->screen_size.h - camera->margin.top - camera->margin.bottom};
    VRect screen_rect = VRect_from_xywh(-screen_size.w / 2.0, -screen_size.h / 2.0,
                                        screen_size.w, screen_size.h);
    screen_rect = VRect_rotate(screen_rect, VPointZero, camera->rotation_radians);
    screen_rect = VRect_bounding_box(screen_rect);
    GfxSize screen_bound_size = {screen_rect.br.x - screen_rect.tl.x,
                                 screen_rect.br.y - screen_rect.tl.y};
    float bound_coef = MIN(screen_size.w / screen_bound_size.w,
                           screen_size.h / screen_bound_size.h);

    camera->scale = MIN((fequal(track_size.w, screen_bound_size.w * bound_coef) ? 1 :
                            screen_bound_size.w * bound_coef / track_size.w),
                        (fequal(track_size.h, screen_bound_size.h * bound_coef) ? 1 :
                            screen_bound_size.h * bound_coef / track_size.h));
}

void Camera_lerp(Camera *camera) {
    camera->tracking.focal = VPoint_add(
        camera->tracking.focal,
        VPoint_scale(VPoint_subtract(camera->focal, camera->tracking.focal),
                     camera->lerp)
    );
    camera->tracking.scale += ((camera->scale - camera->tracking.scale) *
                               camera->lerp);

    // Rotation does not lerp
    camera->tracking.rotation_radians = camera->rotation_radians;

    // Translation does not lerp
    camera->tracking.translation = camera->translation;
}

void Camera_snap_tracking(Camera *camera) {
    camera->tracking.focal = camera->focal;
    camera->tracking.scale = camera->scale;
    camera->tracking.rotation_radians = camera->rotation_radians;
    camera->tracking.translation = camera->translation;
}

void Camera_track(Camera *camera) {
    if (!camera->has_scene_size) {
        camera->scene_size = camera->screen_size;
        VPoint center = {camera->screen_size.w / 2.0,
                         camera->screen_size.h / 2.0};
        camera->focal = center;
        Camera_snap_tracking(camera);
        return;
    }

    if (camera->track_entities && camera->num_entities == 1) {
        Camera_track_single(camera);
    } else if (camera->track_entities && camera->num_entities > 0) {
        Camera_track_multi(camera);
    }

    if (camera->num_entities <= 1) {
        Camera_correct_focal(camera);
    }
    Camera_correct_scale(camera);
    Camera_lerp(camera);
}

void Camera_destroy(Camera *camera) {
    if (camera->track_entities) free(camera->track_entities);
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
            0.0, 256.0); // mediump float max
    Graphics_scale_projection_matrix(graphics, camera->tracking.scale,
            camera->tracking.scale, 1);
    Graphics_rotate_projection_matrix(graphics,
                                      camera->tracking.rotation_radians *
                                          180 / M_PI,
                                      0, 0, -1);
    Graphics_translate_projection_matrix(graphics,
                                         -camera->tracking.focal.x,
                                         -camera->tracking.focal.y,
                                         0);
    Graphics_translate_projection_matrix(graphics,
                                         -camera->tracking.translation.x,
                                         -camera->tracking.translation.y,
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

VRect Camera_visible_rect(Camera *camera, int lerped) {
  VRect cam_rect = Camera_base_rect(camera);

  double scale, rotation;
  VPoint focal, translation;
  if (lerped) {
      scale = camera->tracking.scale;
      rotation = camera->tracking.rotation_radians;
      focal = camera->tracking.focal;
      translation = camera->tracking.translation;
  } else {
      scale = camera->scale;
      rotation = camera->rotation_radians;
      focal = camera->focal;
      translation = camera->translation;
  }

  cam_rect = VRect_scale(cam_rect, 1 / scale);
  cam_rect = VRect_rotate(cam_rect, VRect_center(cam_rect), rotation);
  cam_rect = VRect_move(cam_rect, translation);
  cam_rect = VRect_move(cam_rect, focal);
  return cam_rect;
}

VRect Camera_tracking_rect(Camera *camera) {
  VRect new = VRect_inset(Camera_base_rect(camera), camera->margin);
  return new;
}

VPoint Camera_project_point(Camera *camera, VPoint point,
        int apply_translation, int lerped) {
  double scale, rotation;
  VPoint focal, translation;
  if (lerped) {
      scale = camera->tracking.scale;
      rotation = camera->tracking.rotation_radians;
      focal = camera->tracking.focal;
      translation = camera->tracking.translation;
  } else {
      scale = camera->scale;
      rotation = camera->rotation_radians;
      focal = camera->focal;
      translation = camera->translation;
  }

  VPoint new = VPoint_subtract(point, focal);
  if (apply_translation) {
    new = VPoint_subtract(new, translation);
  }

  new = VPoint_rotate(new, VPointZero, -rotation);
  new = VPoint_scale(new, scale);

  return new;
}

VPoint Camera_cast_point(Camera *camera, VPoint point, int lerped) {
  double scale, rotation;
  VPoint focal, translation;
  if (lerped) {
      scale = camera->tracking.scale;
      rotation = camera->tracking.rotation_radians;
      focal = camera->tracking.focal;
      translation = camera->tracking.translation;
  } else {
      scale = camera->scale;
      rotation = camera->rotation_radians;
      focal = camera->focal;
      translation = camera->translation;
  }

  VPoint correction = {-camera->screen_size.w / 2.0,
                       -camera->screen_size.h / 2.0};
  correction = VPoint_scale(correction, 1 / scale);
  correction = VPoint_rotate(correction, VPointZero, rotation);

  VPoint top_left = VPoint_add(focal, translation);
  top_left = VPoint_add(top_left, correction);

  VPoint new = point;
  new = VPoint_scale(new, 1 / scale);
  new = VPoint_rotate(new, VPointZero, rotation);
  new = VPoint_add(new, top_left);

  return new;
}

VRect Camera_project_rect(Camera *camera, VRect rect,
        int apply_translation, int lerped) {
  VRect new = rect;
  new.tl = Camera_project_point(camera, rect.tl, apply_translation, lerped);
  new.tr = Camera_project_point(camera, rect.tr, apply_translation, lerped);
  new.br = Camera_project_point(camera, rect.br, apply_translation, lerped);
  new.bl = Camera_project_point(camera, rect.bl, apply_translation, lerped);
  return new;
}

void Graphics_project_screen_camera(Graphics *graphics, Camera *camera) {
    Camera screen_cam = {
      .focal = {0, 0},
      .screen_size = camera->screen_size,
      .scale = 1,
      .rotation_radians = 0,
      .margin = camera->margin,
      .translation = {0, 0}
    };
    Camera_snap_tracking(&screen_cam);

    Graphics_reset_projection_matrix(graphics);
    Graphics_project_camera(graphics, &screen_cam);
}

void Camera_debug(Camera *camera, Graphics *graphics) {
    Graphics_project_screen_camera(graphics, camera);
    Graphics_reset_modelview_matrix(graphics);

    Camera screen_cam = {
      .focal = {0, 0},
      .screen_size = camera->screen_size,
      .scale = 1,
      .rotation_radians = 0,
      .margin = camera->margin
    };
    GLfloat cam_color[4] = {1, 0, 0, 1};
    VRect track_rect = Camera_tracking_rect(&screen_cam);
    track_rect = VRect_move(track_rect, VPoint_scale(camera->translation, -camera->scale));
    Graphics_stroke_rect(graphics, track_rect, cam_color, 1, 0, 0);

    if (camera->track_entities) {
        GLfloat e_color[4] = {0, 1, 0, 1};
        Entity **entity = camera->track_entities;
        int i = 0;
        for (i = 0; i < camera->num_entities; i++, entity++) {
            VRect e_rect = Entity_real_rect(*entity);
            VRect e_bound = Camera_project_rect(camera, e_rect, 1, 1);
            e_bound = VRect_bounding_box(e_bound);
            Graphics_stroke_rect(graphics, e_bound, e_color, 1, 0, 0);
        }
    }
}
