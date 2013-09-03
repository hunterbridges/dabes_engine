#include "canvas.h"
#include "../scenes/scene.h"

static const float CANVAS_DEFAULT_ANGLE_THRESH = 30.f;
static const float CANVAS_DEFAULT_DISTANCE_THRESH = 25.f;
static const int CANVAS_DEFAULT_DRAW_WIDTH = 2;

static const VVector4 CANVAS_DEFAULT_DRAW_COLOR = {.raw = {1.f, 1.f, 0.f, 1.f}};
static const VVector4 CANVAS_DEFAULT_BG_COLOR = {.raw = {0.f, 0.f, 0.f, 0.5f}};

Canvas *Canvas_create(Engine *engine) {
    check(engine != NULL, "Engine required");
    Canvas *canvas = calloc(1, sizeof(Canvas));

    canvas->alpha = 1.f;
    canvas->angle_threshold = CANVAS_DEFAULT_ANGLE_THRESH;
    canvas->distance_threshold = CANVAS_DEFAULT_DISTANCE_THRESH;
    canvas->draw_width = CANVAS_DEFAULT_DRAW_WIDTH;

    canvas->draw_color = CANVAS_DEFAULT_DRAW_COLOR;
    canvas->bg_color = CANVAS_DEFAULT_BG_COLOR;

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

    if (canvas->simplified_points) {
        DArray_clear_destroy(canvas->simplified_points);
        canvas->simplified_points = NULL;
    }

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
