#include "canvas.h"
#include "../scenes/scene.h"

static const float CANVAS_DEFAULT_ANGLE_THRESH = 30.f;
static const float CANVAS_DEFAULT_DISTANCE_THRESH = 25.f;
static const int CANVAS_DEFAULT_DRAW_WIDTH = 2;

static const VVector4 CANVAS_DEFAULT_DRAW_COLOR = {.raw = {1.f, 1.f, 0.f, 1.f}};
static const VVector4 CANVAS_DEFAULT_SIMP_COLOR = {.raw = {1.f, 0.f, 0.f, 0.8f}};
static const VVector4 CANVAS_DEFAULT_BG_COLOR = {.raw = {0.f, 0.f, 0.f, 0.5f}};


static void simplifier_stage_point(Canvas *canvas, VPoint p);
static void simplifier_try_point(Canvas *canvas, VPoint p,
                                 short int suppress_commit);
static void simplifier_commit_point(Canvas *canvas);
static void simplifier_empty(Canvas *canvas);
VPoint *simplifier_staged_path(Canvas *canvas, int *num_points);

Canvas *Canvas_create(Engine *engine) {
    check(engine != NULL, "Engine required");
    Canvas *canvas = calloc(1, sizeof(Canvas));

    canvas->alpha = 1.f;
    canvas->angle_threshold = CANVAS_DEFAULT_ANGLE_THRESH;
    canvas->distance_threshold = CANVAS_DEFAULT_DISTANCE_THRESH;
    canvas->draw_width = CANVAS_DEFAULT_DRAW_WIDTH;

    canvas->draw_color = CANVAS_DEFAULT_DRAW_COLOR;
    canvas->bg_color = CANVAS_DEFAULT_BG_COLOR;
    canvas->simplified_path_color = CANVAS_DEFAULT_SIMP_COLOR;

    return canvas;
error:
    return NULL;
}

void Canvas_destroy(Canvas *canvas) {
    check(canvas != NULL, "Canvas required");
    Canvas_empty(canvas);

    return;
error:
    return;
}

void Canvas_empty_queue(Canvas *canvas) {
    check(canvas != NULL, "Canvas required");
    memset(canvas->point_queue, 0, sizeof(VPoint) * CANVAS_QUEUE_SIZE);
    canvas->queue_count = 0;

    return;
error:
    return;
}

void Canvas_empty(Canvas *canvas) {
    check(canvas != NULL, "Canvas required");
    Canvas_empty_queue(canvas);

    if (canvas->raw_points) {
        DArray_clear_destroy(canvas->raw_points);
        canvas->raw_points = NULL;
    }

    simplifier_empty(canvas);

    return;
error:
    return;
}

void Canvas_enqueue_point(Canvas *canvas, VPoint point) {
    check(canvas != NULL, "Canvas required");
    if (canvas->queue_count >= CANVAS_QUEUE_SIZE) {
        log_warn("Trying to enqueue a point when we already have %d",
                 canvas->queue_count);
        return;
    }

    canvas->point_queue[canvas->queue_count] = point;
    canvas->queue_count++;

    return;
error:
    return;
}

void Canvas_consume_queue(Canvas *canvas) {
    check(canvas != NULL, "Canvas required");

    if (canvas->queue_count == 0) return;

    if (canvas->raw_points == NULL) {
        canvas->raw_points = DArray_create(sizeof(VPoint), CANVAS_QUEUE_SIZE);
    }

    int i = 0;
    for (i = 0; i < canvas->queue_count; i++) {
        VPoint *to_save = calloc(1, sizeof(VPoint));
        *to_save = canvas->point_queue[i];
        DArray_push(canvas->raw_points, to_save);

        if (canvas->simplified_points) {
            simplifier_try_point(canvas, canvas->point_queue[i], 0);
        } else {
            simplifier_stage_point(canvas, canvas->point_queue[i]);
            simplifier_commit_point(canvas);

            if (canvas->shape_matcher) {
                ShapeMatcher_start(canvas->shape_matcher,
                        canvas->point_queue[i]);
            }
        }
    }

    Canvas_empty_queue(canvas);
    return;
error:
    return;
}

void Canvas_update(Canvas *canvas, Engine *engine) {
    check(canvas != NULL, "Canvas required");

    Controller *p1 = engine->input->controllers[0];
    short int hold = !!(p1->touch_state & CONTROLLER_TOUCH_HOLD);
    short int hold_changed = !!(p1->touch_state & CONTROLLER_TOUCH_HOLD_CHANGED);
    short int moved = !!(p1->touch_state & CONTROLLER_TOUCH_MOVED);
    if (hold && hold_changed) {
        Canvas_empty(canvas);

        if (canvas->shape_matcher) {
            ShapeMatcher_reset(canvas->shape_matcher);
        }
    }

    if (hold && (hold_changed || moved)) {
        Canvas_enqueue_point(canvas, p1->touch_pos);
    }

    Canvas_consume_queue(canvas);

    return;
error:
    return;
}

void Canvas_render(Canvas *canvas, Engine *engine) {
    check(canvas != NULL, "Canvas required");
    check(canvas->scene != NULL, "Canvas is not in a scene");

    Graphics_reset_modelview_matrix(engine->graphics);
    Scene_project_screen(canvas->scene, engine);
    VPoint screen_diff = {canvas->scene->camera->screen_size.w / 2.0,
                          canvas->scene->camera->screen_size.h / 2.0};
    VRect bg_rect =
        VRect_from_xywh(-screen_diff.x,
                        -screen_diff.y,
                        canvas->scene->camera->screen_size.w,
                        canvas->scene->camera->screen_size.h);
    glUniformMatrix4fv(GfxShader_uniforms[UNIFORM_DECAL_PROJECTION_MATRIX], 1,
                       GL_FALSE, engine->graphics->projection_matrix.gl);
    Graphics_draw_rect(engine->graphics, NULL, bg_rect, canvas->bg_color.raw,
                       NULL, VPointZero, GfxSizeZero, 0, 1, canvas->alpha);

    if (canvas->raw_points) {
        // Adjust each point cause canvas points are relative to screen top left
        // and projection matrix thinks <0, 0> is screen center
        int num_points = DArray_count(canvas->raw_points);
        VPoint *path =
            malloc(sizeof(VPoint) * num_points);
        int i = 0;
        for (i = 0; i < num_points; i++) {
            VPoint *point = DArray_get(canvas->raw_points, i);
            path[i] = VPoint_subtract(*point, screen_diff);
        }

        VVector4 path_draw_color = canvas->draw_color;
        path_draw_color.rgba.a *= canvas->alpha;

        Graphics_stroke_path(engine->graphics, path, num_points,
                             VPointZero, path_draw_color.raw,
                             canvas->draw_width, 0, 0);

        free(path);
    }

    int num_staged;
    VPoint *staged_path = simplifier_staged_path(canvas, &num_staged);
    if (staged_path) {
        int i = 0;
        for (i = 0; i < num_staged; i++) {
            VPoint point = staged_path[i];
            staged_path[i] = VPoint_subtract(point, screen_diff);
        }

        VVector4 path_draw_color = canvas->simplified_path_color;
        path_draw_color.rgba.a *= canvas->alpha;

        Graphics_stroke_path(engine->graphics, staged_path, num_staged,
                             VPointZero, path_draw_color.raw,
                             canvas->draw_width, 0, 0);

        free(staged_path);
    }

    if (canvas->shape_matcher) {
        // TODO Render shape matcher shtuff
    }

    return;
error:
    return;
}

void Canvas_set_enabled(Canvas *canvas, Engine *engine, int enabled) {
    check(canvas != NULL, "Canvas required");

    if (enabled) {
        Input_change_preferred_style(engine->input, INPUT_STYLE_TOUCHPAD);
    } else {
        Input_change_preferred_style(engine->input, INPUT_STYLE_LEFT_RIGHT);
    }

    return;
error:
    return;
}

#pragma mark - Simplifier

static void simplifier_stage_point(Canvas *canvas, VPoint p) {
    if (canvas->staged_point) {
        free(canvas->staged_point);
    }

    canvas->staged_point = malloc(sizeof(Point));
    *canvas->staged_point = p;

    if (canvas->shape_matcher) {
        ShapeMatcher_stage_point(canvas->shape_matcher, p);
    }
}

static void simplifier_try_point(Canvas *canvas, VPoint p,
                                 short int suppress_commit) {
    short int has_one_ago = 0;
    VPoint one_ago = VPointZero;
    if (canvas->staged_point) {
        has_one_ago = 1;
        one_ago = *canvas->staged_point;
    }

    short int has_two_ago = 0;
    VPoint two_ago = VPointZero;
    int simp_count = (canvas->simplified_points ?
                      DArray_count(canvas->simplified_points) :
                      0);
    if (simp_count) {
        has_two_ago = 1;
        VPoint *got = DArray_get(canvas->simplified_points, simp_count - 1);
        two_ago = *got;
    }

    short int distance_ok = 0;
    short int angle_ok = 0;

    if (has_two_ago) {
        float dist = VPoint_magnitude(VPoint_subtract(p, two_ago));
        if (dist > canvas->distance_threshold) {
            distance_ok = 1;
        }
    }

    if (has_one_ago && has_two_ago) {
        float before_angle = VPoint_angle(two_ago, one_ago);
        float this_angle = VPoint_angle(one_ago, p);
        float diff = fabsf(this_angle - before_angle);
        float diff_deg = diff * 180.f / M_PI;
        if (diff_deg > canvas->angle_threshold) {
            angle_ok = 1;
        }
    }

    if (!suppress_commit && (distance_ok && angle_ok)) {
        simplifier_commit_point(canvas);
    }

    simplifier_stage_point(canvas, p);
}

static void simplifier_commit_point(Canvas *canvas) {
    if (!canvas->staged_point) return;

    if (!canvas->simplified_points) {
        canvas->simplified_points =
            DArray_create(sizeof(VPoint), CANVAS_QUEUE_SIZE);
    }

    DArray_push(canvas->simplified_points, canvas->staged_point);
    canvas->staged_point = NULL;

    if (canvas->shape_matcher) {
        ShapeMatcher_commit_point(canvas->shape_matcher);
    }
}

static void simplifier_empty(Canvas *canvas) {
    if (canvas->simplified_points) {
        free(canvas->simplified_points);
        canvas->simplified_points = NULL;
    }

    if (canvas->staged_point) {
        free(canvas->staged_point);
        canvas->staged_point = NULL;
    }
}

VPoint *simplifier_staged_path(Canvas *canvas, int *num_points) {
    check(num_points != NULL, "*num_points can't be NULL");

    if (canvas->simplified_points == NULL) {
        *num_points = 0;
        return NULL;
    }

    int num = DArray_count(canvas->simplified_points);
    int simplified_count = num;
    if (canvas->staged_point) {
        num++;
    }
    *num_points = num;

    if (num == 0) {
        return NULL;
    }

    VPoint *points = calloc(num, sizeof(VPoint));
    int i = 0;
    for (i = 0; i < num; i++) {
        if (i < simplified_count) {
            // Get it out of simplified
            VPoint *simp_point = DArray_get(canvas->simplified_points, i);
            points[i] = *simp_point;
        } else {
            // It's the staged point
            points[i] = *canvas->staged_point;
        }
    }

    return points;
error:
    return NULL;
}
