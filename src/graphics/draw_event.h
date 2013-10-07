#ifndef __draw_event_h
#define __draw_event_h
#include "../prefix.h"

typedef enum {
    DRAW_EVENT_ENTITY = 1,
    DRAW_EVENT_CANVAS_BG,
    DRAW_EVENT_CANVAS_PATHS,
    DRAW_EVENT_TILEMAP_LAYER,
    DRAW_EVENT_PARALLAX_BG,
    DRAW_EVENT_PARALLAX_LAYER,
    DRAW_EVENT_OVERLAY_STRING,
    DRAW_EVENT_OVERLAY_SPRITE,
    DRAW_EVENT_SCENE_FILL
} DrawEventKind;

struct Graphics;
struct GfxShader;
struct DrawEvent;

typedef void (*DrawEventFunc)(struct DrawEvent *event,
                              struct Graphics *graphics);

typedef struct DrawEvent {
    DrawEventKind kind;
    float z;
    short int opaque;

    struct GfxShader *shader;

    DrawEventFunc func;
    void *context;
} DrawEvent;

int DrawEvent_cmp(const void *a, const void *b);
DrawEvent *DrawEvent_create(DrawEventKind kind, float z,
        struct GfxShader *shader);
void DrawEvent_destroy(DrawEvent *event);
void DrawEvent_draw(DrawEvent *event, struct Graphics *graphics);

#endif
