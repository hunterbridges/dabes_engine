#ifndef __graphics_h
#define __graphics_h
#include "../prefix.h"

typedef struct GfxPoint {
    double x;
    double y;
} GfxPoint;

typedef struct GfxRect {
    GfxPoint tl;
    GfxPoint tr;
    GfxPoint bl; //NOTICE bl AND br ARE FLIPPED FROM PhysBox!
    GfxPoint br;
} GfxRect;

GfxRect GfxRect_from_xywh(double x, double y, double w, double h);
GfxRect GfxRect_from_SDL_Rect(SDL_Rect rect);

typedef struct Graphics {
    Object proto;
} Graphics;

void Graphics_draw_rect(Graphics *graphics, GfxRect rect, GLdouble color[4],
        GLuint texture, double rotation);
int Graphics_init(void *self);

extern Object GraphicsProto;

#endif
