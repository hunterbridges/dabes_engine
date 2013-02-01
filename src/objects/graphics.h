#ifndef __graphics_h
#define __graphics_h
#include "../prefix.h"

typedef struct GfxPoint {
    float x;
    float y;
} GfxPoint;

typedef struct GfxRect {
    GfxPoint tl;
    GfxPoint tr;
    GfxPoint bl; //NOTICE bl AND br ARE FLIPPED FROM PhysBox!
    GfxPoint br;
} GfxRect;

GfxRect GfxRect_from_xywh(float x, float y, float w, float h);
GfxRect GfxRect_from_SDL_Rect(SDL_Rect rect);

typedef struct Graphics {
    Object proto;
} Graphics;

void Graphics_draw_rect(Graphics *graphics, GfxRect rect, GLfloat color[4],
        GLuint texture, float rotation);
int Graphics_init(void *self);

extern Object GraphicsProto;

#endif
