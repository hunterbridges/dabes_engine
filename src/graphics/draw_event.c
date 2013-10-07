#include "draw_event.h"
#include "graphics.h"

int DrawEvent_cmp(const void *a, const void *b) {
    DrawEvent *ev_a = *(DrawEvent **)a;
    DrawEvent *ev_b = *(DrawEvent **)b;

    if (ev_a->z < ev_b->z) return -1;
    if (ev_a->z > ev_b->z) return 1;
    return 0;
}

DrawEvent *DrawEvent_create(DrawEventKind kind, float z, GfxShader *shader) {
    DrawEvent *event = NULL;
    check(shader != NULL, "DrawEvent requires a GfxShader");

    event = calloc(1, sizeof(DrawEvent));
    check(event != NULL, "Couldn't create DrawEvent");

    event->kind = kind;
    event->z = z;
    event->shader = shader;

    return event;
error:
    if (event) {
        free(event);
    }
    return NULL;
}

void DrawEvent_destroy(DrawEvent *event) {
    check(event != NULL, "No event to destroy");

    if (event->context) {
        free(event->context);
    }

    free(event);

    return;
error:
    return;
}

void DrawEvent_draw(DrawEvent *event, Graphics *graphics) {
    check(event != NULL, "Event required");
    check(graphics != NULL, "Graphics required");

    Graphics_use_shader(graphics, event->shader);
    event->func(event, graphics);

    return;
error:
    return;
}
