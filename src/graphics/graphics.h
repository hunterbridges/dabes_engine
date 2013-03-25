#ifndef __graphics_h
#define __graphics_h
#include "../prefix.h"

#ifdef DABES_IOS
#include <GLKit/GLKMath.h>
#include <OpenGLES/ES2/gl.h>
#include <QuartzCore/CoreAnimation.h>
#else
#include <SDL/SDL_ttf.h>
#endif

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
GfxRect GfxRect_fill_size(GfxSize source_size, GfxSize dest_size);
GfxRect GfxRect_from_SDL_Rect(SDL_Rect rect);

typedef struct GfxTransform3D {
    float m11, m12, m13, m14;
    float m21, m22, m23, m24;
    float m31, m32, m33, m34;
    float m41, m42, m43, m44;
} GfxTransform3D;

GfxTransform3D GfxTransform3D_ortho(float left, float right, float top,
                                    float bottom, float near, float far);
typedef union GfxUVertex {
  struct {
    GLfloat x;
    GLfloat y;
    GLfloat z;
    GLfloat w;
  } packed;
  float raw[4];
} GfxUVertex;

typedef union GfxUMatrix {
    GfxTransform3D gfx;
    float gl[16];
#ifdef DABES_IOS
    CATransform3D ca;
    GLKMatrix4 glk;
#endif
} GfxUMatrix;

typedef struct Graphics {
    Object proto;
    GfxSize screen_size;
    GLuint debug_text_texture;
#ifndef DABES_IOS
    TTF_Font *debug_text_font;
#endif
    GLuint shader;
    GLuint array_buffer;

    GfxUMatrix projection_matrix;
    GfxUMatrix modelview_matrix;
} Graphics;

void Graphics_stroke_rect(Graphics *graphics, GfxRect rect, GLfloat color[4],
        double line_width);
void Graphics_draw_rect(Graphics *graphics, GfxRect rect, GLfloat color[4],
        GLuint texture, double rotation);
void Graphics_draw_debug_text(Graphics *graphics,
        int ticks_since_last);

void Graphics_reset_projection_matrix(Graphics *graphics);
void Graphics_ortho_projection_matrix(Graphics *graphics, double left,
        double right, double top, double bottom, double far, double near);
void Graphics_scale_projection_matrix(Graphics *graphics, double x,
        double y, double z);
void Graphics_rotate_projection_matrix(Graphics *graphics, double rot_degs,
                                       double x, double y, double z);
void Graphics_translate_projection_matrix(Graphics *graphics,
                                         double x, double y, double z);

void Graphics_reset_modelview_matrix(Graphics *graphics);
void Graphics_scale_modelview_matrix(Graphics *graphics,
                                     double x, double y, double z);
void Graphics_rotate_modelview_matrix(Graphics *graphics, double rot_degs,
                                       double x, double y, double z);
void Graphics_translate_modelview_matrix(Graphics *graphics,
                                         double x, double y, double z);

GLuint Graphics_load_shader(Graphics *graphics, char *vert_name,
        char *frag_name);
int Graphics_init(void *self);
void Graphics_log_shader(GLuint shader);
void Graphics_log_program(GLuint program);

extern Object GraphicsProto;

#endif
