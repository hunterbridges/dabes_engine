#ifndef __graphics_h
#define __graphics_h
#include "../prefix.h"
#include "SDL/SDL_TTF.h"

typedef struct GfxPoint {
    double x;
    double y;
} GfxPoint;

typedef struct GfxSize {
    double w;
    double h;
} GfxSize;

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
    GfxSize screen_size;
    GLuint debug_text_texture;
    TTF_Font *debug_text_font;
    GLuint shader;
} Graphics;

void Graphics_stroke_rect(Graphics *graphics, GfxRect rect, GLdouble color[4],
        double line_width);
void Graphics_draw_rect(Graphics *graphics, GfxRect rect, GLdouble color[4],
        GLuint texture, double rotation);
void Graphics_draw_debug_text(Graphics *graphics,
        long unsigned int ticks_since_last);
void Graphics_scale_projection_matrix(Graphics *graphics, double scale);
GLuint Graphics_load_shader(Graphics *graphics, char *vert_name,
        char *frag_name);
int Graphics_init(void *self);
void Graphics_log_shader(GLuint shader);
void Graphics_log_program(GLuint program);

extern Object GraphicsProto;

#endif
