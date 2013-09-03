#ifndef __canvas_h
#define __canvas_h
#include "../prefix.h"
#include <lcthw/darray.h>
#include "../core/engine.h"
#include "../math/vmatrix.h"

struct Scene;

#define CANVAS_QUEUE_SIZE 128

typedef struct Canvas {
    DArray *raw_points;
    DArray *simplified_points;

    int enabled;
    float alpha;
    VPoint offset;

    struct Scene *scene;

    float angle_threshold;
    float distance_threshold;
    float draw_width;

    VVector4 bg_color;
    VVector4 draw_color;
    VVector4 simplified_path_color;

    VPoint point_queue[CANVAS_QUEUE_SIZE];
    int queue_count;
    
    VPoint *staged_point;
} Canvas;

Canvas *Canvas_create(Engine *engine);
void Canvas_destroy(Canvas *canvas);
void Canvas_empty(Canvas *canvas);
void Canvas_enqueue_point(Canvas *canvas, VPoint point);
void Canvas_update(Canvas *canvas, Engine *engine);
void Canvas_render(Canvas *canvas, Engine *engine);
void Canvas_set_enabled(Canvas *canvas, Engine *engine, int enabled);

#endif
