#ifndef __graphics_h
#define __graphics_h
#include "../prefix.h"

typedef struct Graphics {
    Object proto;
    void (*draw_rect)(void *self, SDL_Rect rect, GLfloat *color,
            GLuint texture, float rotation);
} Graphics;


void Graphics_draw_rect(void *self, SDL_Rect rect, GLfloat *color, GLuint texture,
        float rotation);
int Graphics_init(void *self);

extern Object GraphicsProto;

#endif
