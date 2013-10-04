#ifndef __graphics_h
#define __graphics_h
#include <lcthw/hashmap.h>
#include "../prefix.h"
#include "../math/vmatrix.h"
#include "../math/vrect.h"
#include "../math/vcircle.h"

#ifdef DABES_SDL
#include <SDL/SDL_ttf.h>
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

extern GLfloat GfxGLClearColor[4];

struct Graphics;

int Graphics_init_GL(int swidth, int sheight);

typedef struct GfxSize {
    double w;
    double h;
} GfxSize;

static const GfxSize GfxSizeZero = {0,0};
VRect VRect_fill_size(GfxSize source_size, GfxSize dest_size);

////////////////////////////////////////////////////////////////////////////////

typedef struct GfxTexture {
    const char *name;
    GfxSize size;
    GfxSize pot_size;
    GLuint gl_tex;
} GfxTexture;

GfxTexture *GfxTexture_from_data(unsigned char **data, int width, int height,
        GLenum source_format, int mipmap, size_t size);
GfxTexture *GfxTexture_from_image(const char *image_name, int mipmap);
void GfxTexture_destroy(GfxTexture *texture);

////////////////////////////////////////////////////////////////////////////////

typedef enum {
    GfxTextAlignLeft = 0,
    GfxTextAlignRight = 1,
    GfxTextAlignCenter = 2
} GfxTextAlign;

typedef struct GfxFontChar {
    GfxTexture *texture;
    FT_Vector advance;
    float bitmap_top;
} GfxFontChar;

GfxFontChar *GfxFontChar_create(FT_Bitmap *bitmap, FT_Vector advance,
                                float bitmap_top);
void GfxFontChar_destroy(GfxFontChar *fontchar);

typedef struct GfxFont {
    FT_Face face;
    char *name;
    int px_size;
    Hashmap *char_textures;
} GfxFont;

GfxFont *GfxFont_create(struct Graphics *graphics, const char *font_name, int px_size);
void GfxFont_destroy(GfxFont *font);
GfxFontChar *GfxFont_get_char(GfxFont *font, char c, int stroke,
                              struct Graphics *graphics);

////////////////////////////////////////////////////////////////////////////////

typedef enum {
    UNIFORM_DECAL_PROJECTION_MATRIX,
    UNIFORM_DECAL_TEXTURE,
    UNIFORM_DECAL_TEX_ALPHA_ADJ,
    UNIFORM_TILEMAP_PROJECTION_MATRIX,
    UNIFORM_TILEMAP_MODELVIEW_MATRIX,
    UNIFORM_TILEMAP_TILE_SIZE,
    UNIFORM_TILEMAP_SHEET_ROWS_COLS,
    UNIFORM_TILEMAP_SHEET_PORTION,
    UNIFORM_TILEMAP_SHEET_POT_SIZE,
    UNIFORM_TILEMAP_MAP_ROWS_COLS,
    UNIFORM_TILEMAP_TEXEL_PER_MAP,
    UNIFORM_TILEMAP_ATLAS,
    UNIFORM_TILEMAP_TILESET,
    UNIFORM_PARALLAX_PROJECTION_MATRIX,
    UNIFORM_PARALLAX_MODELVIEW_MATRIX,
    UNIFORM_PARALLAX_TEXTURE,
    UNIFORM_PARALLAX_TEX_PORTION,
    UNIFORM_PARALLAX_STRETCH,
    UNIFORM_PARALLAX_CASCADE_TOP,
    UNIFORM_PARALLAX_CASCADE_BOTTOM,
    UNIFORM_PARALLAX_ORIG_PIXEL,
    UNIFORM_PARALLAX_REPEAT_SIZE,
    UNIFORM_PARALLAX_REPEATS,
    UNIFORM_PARALLAX_X_SHIFT,
    UNIFORM_PARALLAX_CAMERA_POS,
    UNIFORM_PARALLAX_FACTOR,
    UNIFORM_PARALLAX_TEX_SCALE,
    UNIFORM_TEXT_PROJECTION_MATRIX,
    UNIFORM_TEXT_MODELVIEW_MATRIX,
    UNIFORM_TEXT_TEXTURE,
    UNIFORM_TEXT_STRETCH,
    UNIFORM_TEXT_COLOR,
    NUM_UNIFORMS
} GraphicsUniform;

enum {
    ATTRIB_DECAL_VERTEX,
    ATTRIB_DECAL_COLOR,
    ATTRIB_DECAL_ALPHA,
    ATTRIB_DECAL_TEXTURE,
    ATTRIB_DECAL_MODELVIEW_MATRIX,
    ATTRIB_TILEMAP_VERTEX,
    ATTRIB_TILEMAP_TEXTURE,
    ATTRIB_PARALLAX_VERTEX,
    ATTRIB_PARALLAX_TEXTURE,
    ATTRIB_TEXT_VERTEX,
    ATTRIB_TEXT_TEX_POS,
	NUM_ATTRIBUTES
} ATTRIBS;

enum {
  SAMPLER_TILEMAP_ATLAS,
  SAMPLER_TILEMAP_TILESET,
  NUM_SAMPLERS
} SAMPLERS;

extern GLint GfxShader_uniforms[NUM_UNIFORMS];
extern GLint GfxShader_attributes[NUM_ATTRIBUTES];
extern GLint GfxShader_samplers[NUM_SAMPLERS];

struct Graphics;
struct DrawBuffer;
typedef struct GfxShader {
    void (*set_up)(struct GfxShader *shader, struct Graphics *graphics);
    void (*tear_down)(struct GfxShader *shader, struct Graphics *graphics);
    GLuint gl_program;
    GLuint gl_vertex_array;
    GLuint gl_vertex_buffer;
    struct DrawBuffer *draw_buffer;
} GfxShader;

void GfxShader_destroy(GfxShader *shader, struct Graphics *graphics);

////////////////////////////////////////////////////////////////////////////////

typedef struct Graphics {
    struct Engine *engine;

    FT_Library ft;

    GfxSize screen_size;
    GfxShader *current_shader;
    GfxFont *debug_font;
    GLuint array_buffer;

    VMatrix projection_matrix;
    VMatrix modelview_matrix;

    Hashmap *textures;
    Hashmap *shaders;
    List *shader_list;
    Hashmap *sprites;
  
    int num_uniforms;
    void **uniforms;

    int gl_vao_enabled;
    void (*gen_vao)(GLsizei n, GLuint *arrays);
    void (*bind_vao)(GLuint array);
    void (*del_vao)(GLsizei n, const GLuint *arrays);
} Graphics;

struct Engine;
Graphics *Graphics_create(struct Engine *engine);
void Graphics_destroy(Graphics *graphics);

void Graphics_stroke_path(Graphics *graphics, VPoint *points, int num_points,
        VPoint center, GLfloat color[4], double line_width, double rotation,
        int loop);
void Graphics_stroke_circle(Graphics *graphics, VCircle circle, int precision,
        VPoint center, GLfloat color[4], double line_width);
void Graphics_stroke_rect(Graphics *graphics, VRect rect, GLfloat color[4],
        double line_width, double rotation);
void Graphics_draw_rect(Graphics *graphics, struct DrawBuffer *draw_buffer,
        VRect rect, GLfloat color[4], GfxTexture *texture, VPoint textureOffset,
        GfxSize textureSize, double rotation, int z_index, GLfloat alpha);
void Graphics_draw_string(Graphics *graphics, char *text, GfxFont *font,
        GLfloat color[4], VPoint origin, GfxTextAlign align,
        GLfloat shadow_color[4], VPoint shadow_offset);

// Uniforms
void Graphics_get_uniform(Graphics *graphics, GLint program, const GLchar *name,
                          GraphicsUniform uniform);
void Graphics_clear_uniforms(Graphics *graphics);
void Graphics_uniformMatrix4fv(Graphics *graphics, GraphicsUniform uniform,
                               GLfloat vector[16], GLboolean transpose);
void Graphics_uniform1i(Graphics *graphics, GraphicsUniform uniform,
                        GLint x);
void Graphics_uniform1f(Graphics *graphics, GraphicsUniform uniform,
                        GLfloat x);
void Graphics_uniform2f(Graphics *graphics, GraphicsUniform uniform,
                        GLfloat x, GLfloat y);
void Graphics_uniform4f(Graphics *graphics, GraphicsUniform uniform,
                        GLfloat x, GLfloat y, GLfloat z, GLfloat w);


// Projection matrix
void Graphics_reset_projection_matrix(Graphics *graphics);
void Graphics_ortho_projection_matrix(Graphics *graphics, double left,
        double right, double top, double bottom, double near, double far);
void Graphics_perspective_projection_matrix(Graphics *graphics,
                                            float fov_radians, float aspect,
                                            float near, float far);
void Graphics_frustum_projection_matrix(Graphics *graphics, double left,
                                        double right, double top, double bottom, double near, double far);
void Graphics_scale_projection_matrix(Graphics *graphics, double x,
        double y, double z);
void Graphics_rotate_projection_matrix(Graphics *graphics, double rot_degs,
                                       double x, double y, double z);
void Graphics_translate_projection_matrix(Graphics *graphics,
                                         double x, double y, double z);

// Modelview matrix
void Graphics_reset_modelview_matrix(Graphics *graphics);
void Graphics_scale_modelview_matrix(Graphics *graphics,
                                     double x, double y, double z);
void Graphics_rotate_modelview_matrix(Graphics *graphics, double rot_degs,
                                       double x, double y, double z);
void Graphics_translate_modelview_matrix(Graphics *graphics,
                                         double x, double y, double z);

// Shader
GLuint Graphics_load_shader(Graphics *graphics, char *vert_name,
        char *frag_name, GLuint *compiled_program);
GfxShader *Graphics_get_shader(Graphics *graphics, char *name);
void Graphics_use_shader(Graphics *graphics, GfxShader *shader);
int Graphics_init(void *self);
void Graphics_log_shader(GLuint shader);
void Graphics_log_program(GLuint program);

// Textures
GfxTexture *Graphics_texture_from_image(Graphics *graphics, const char *image_name, int mipmap);

// Sprites
struct Sprite;
void Graphics_draw_sprite(Graphics *graphics, struct Sprite *sprite,
                          struct DrawBuffer *draw_buffer, VRect rect,
                          GLfloat color[4], double rot_degs, int z_index,
                          GLfloat alpha);
struct Sprite *Graphics_sprite_from_image(Graphics *graphics, const char *image_name,
    GfxSize cell_size, int padding);

extern Object GraphicsProto;

#endif
