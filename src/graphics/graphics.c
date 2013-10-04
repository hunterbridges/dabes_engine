#ifdef DABES_IOS
#include <OpenGLES/ES2/glext.h>
#endif
#ifdef DABES_MAC
#include <OpenGL/glext.h>
#endif

#include <freetype/ftglyph.h>
#include <freetype/ftstroke.h>
#include <lcthw/hashmap_algos.h>
#include <lcthw/bstrlib.h>
#include "draw_buffer.h"
#include "graphics.h"
#include "stb_image.h"
#include "sprite.h"
#include "../core/engine.h"

GLfloat GfxGLClearColor[4] = {0.0, 0.0, 0.0, 0.0};

GLint GfxShader_uniforms[NUM_UNIFORMS];
GLint GfxShader_attributes[NUM_ATTRIBUTES];

static inline GLenum Graphics_check() {
    GLenum er = glGetError();
    switch (er) {
      case GL_INVALID_OPERATION:
        log_err("OpenGL GL_INVALID_OPERATION");
        break;

      case GL_INVALID_VALUE:
        log_err("OpenGL GL_INVALID_VALUE");
        break;

      default:
        break;
    }
    return er;
}

int Graphics_init_GL(int UNUSED(swidth), int UNUSED(sheight)) {
    GLenum error = glGetError();
    glEnable(GL_BLEND);
    error = glGetError();

#if defined(DABES_MAC) || defined(DABES_SDL)
    glEnable(GL_MULTISAMPLE);
    glDisable(GL_ALPHA_TEST);
    glDisable(GL_FOG);
#endif
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_DITHER);
    glDisable(GL_STENCIL_TEST);

    error = glGetError();
    check(error == GL_NO_ERROR, "OpenGL init error...");
    return 1;
error:
#ifdef DABES_SDL
    printf("Error initializing OpenGL! %s\n", gluErrorString(error));
#endif
    return 0;
}

// GFX
VRect VRect_fill_size(GfxSize source_size, GfxSize dest_size) {
    double x, y, w, h;
    double source_ratio = source_size.w / source_size.h;
    double dest_ratio = dest_size.w / dest_size.h;
    double scale = 1;
    if (source_ratio < dest_ratio) {
        scale = source_size.w / dest_size.w;
    } else {
        scale = source_size.h / dest_size.h;
    }

    w = source_size.w / scale;
    h = source_size.h / scale;
    x = (w - dest_size.w) / -2;
    y = (h - dest_size.h) / -2;
    return VRect_from_xywh(x, y, w, h);
}

////////////////////////////////////////////////////////////////////////////////

#pragma mark - GfxTexture

static inline int data_potize(unsigned char **data, int width, int height,
                              int *pot_width, int *pot_height, int components,
                              size_t size) {
  int pot_w = 2;
  int pot_h = 2;
  while (pot_w < width) pot_w <<= 1;
  while (pot_h < height) pot_h <<= 1;

  int pot_d = MAX(pot_h, pot_w);

  unsigned char *old_data = *data;
  unsigned char *resized_data = NULL;
  resized_data = realloc(old_data, pot_d * pot_d * sizeof(unsigned char) * 4 * size);
  check(resized_data != NULL, "Couldn't realloc texture for potize");

  *data = resized_data;
  unsigned char *new_data = resized_data;

  *pot_width = pot_d;
  *pot_height = pot_d;

  long int last_orig_index = (width * height - 1) * components;
  long int i = 0;
  for (i = last_orig_index; i >= 0; i -= components) {
    long int idx = i / components;
    long int old_col = idx % width;
    long int old_row = idx / width;
    long int new_index = (old_row * pot_d + old_col) * 4;

    memmove(new_data + (new_index) * size, new_data + (i) * size, size); // R
    if (components > 1) {
      memmove(new_data + (new_index + 1) * size,
              new_data + (i + 1) * size, size); // G
    }
    if (components > 2) {
      memmove(new_data + (new_index + 2) * size,
              new_data + (i + 2) * size, size); // B
    }
    if (components > 3) {
      memmove(new_data + (new_index + 3) * size,
              new_data + (i + 3) * size, size); // A
    }
  }

  return 1;
error:
  return 0;
}

GfxTexture *GfxTexture_from_data(unsigned char **data, int width, int height,
        GLenum source_format, int mipmap, size_t size) {
    GfxTexture *texture = calloc(1, sizeof(GfxTexture));
    check(texture != NULL, "Couldn't not allocate texture");
    texture->size.w = width;
    texture->size.h = height;

    int pot_width, pot_height;
    int num_components = 4;
    if (source_format == GL_LUMINANCE) {
      num_components = 1;
    } else if (source_format == GL_RGB) {
      num_components = 3;
    }
    int rc = data_potize(data, width, height, &pot_width, &pot_height, num_components,
                         size);
    check(rc == 1, "Could not potize data");

    texture->pot_size.w = pot_width;
    texture->pot_size.h = pot_height;

    GLenum er = Graphics_check();
    glGenTextures(1, &texture->gl_tex);
    er = Graphics_check();
    glBindTexture(GL_TEXTURE_2D, texture->gl_tex);
  
    GLenum color_format = GL_RGBA;
    GLenum type = size == sizeof(float) ? GL_FLOAT : GL_UNSIGNED_BYTE;
  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
#if defined(DABES_IOS)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL_APPLE, mipmap ? 8 : 0);
#endif
#if !defined(DABES_IOS)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, mipmap ? 8 : 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
#endif
  
#if defined(DABES_IOS) || defined(DABES_MAC)
    if (mipmap) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    }
#endif

    glTexImage2D(GL_TEXTURE_2D, 0, color_format, pot_width,
                 pot_height, 0, GL_RGBA,
                 type, *data);
#if defined(DABES_IOS) || defined(DABES_MAC)
    if (mipmap) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }
#endif
    er = Graphics_check();
    check(er == GL_NO_ERROR, "Error loading texture: %d", er);
    return texture;
error:
    if (texture) {
        glDeleteTextures(1, &texture->gl_tex);
        free(texture);
    }
    return NULL;
}

GfxTexture *GfxTexture_from_image(const char *image_name, int mipmap) {
    int x, y, n;
    unsigned char *data = stbi_load(image_name, &x, &y, &n, 4);
    GfxTexture *texture =
        GfxTexture_from_data(&data, x, y, GL_RGBA, mipmap, sizeof(char));
    stbi_image_free(data);
    return texture;
}

void GfxTexture_destroy(GfxTexture *texture) {
  assert(texture != NULL);

  GLuint gl_textures[] = {texture->gl_tex};
  glDeleteTextures(1, gl_textures);

  free(texture);
}

////////////////////////////////////////////////////////////////////////////////

#pragma mark - GfxFont

GfxFontChar *GfxFontChar_create(FT_Bitmap *bitmap, FT_Vector advance, float bitmap_top) {
    GfxFontChar *fontchar = NULL;
    check(bitmap != NULL, "Bitmap required");
    fontchar = calloc(1, sizeof(GfxFontChar));
    check(fontchar != NULL, "Couldn't create fontchar");

    size_t sz = bitmap->width * bitmap->rows * sizeof(uint8_t);
    uint8_t *buf = calloc(1, sz);
    memcpy(buf, bitmap->buffer, sz);
    fontchar->texture =
        GfxTexture_from_data(&buf, bitmap->width, bitmap->rows, GL_LUMINANCE, 1,
                             sizeof(char));

    fontchar->advance = advance;
    fontchar->bitmap_top = bitmap_top;

    return fontchar;
error:
    if (fontchar) free(fontchar);
    return NULL;
}

void GfxFontChar_destroy(GfxFontChar *fontchar) {
    check(fontchar != NULL, "No fontchar to destroy");

    GfxTexture_destroy(fontchar->texture);
    free(fontchar);

error:
    return;
}

GfxFont *GfxFont_create(Graphics *graphics, const char *font_name, int px_size) {
    GfxFont *font = NULL;
    check(graphics != NULL, "Graphics required");
    check(graphics->ft != NULL, "FreeType required");

    font = calloc(1, sizeof(GfxFont));
    check(font != NULL, "Couldn't create font");

    int rc = FT_New_Face(graphics->ft, font_name, 0, &font->face);
    check(rc == 0, "Could not open font");

    FT_Set_Pixel_Sizes(font->face, 0, px_size);
    font->px_size = px_size;
    font->name = calloc(strlen(font_name) + 1, sizeof(char));
    strcpy(font->name, font_name);

    font->char_textures = Hashmap_create(NULL, Hashmap_fnv1a_hash);

    return font;
error:
    if (font) free(font);
    return NULL;
}

void GfxFont_destroy(GfxFont *font) {
    check(font != NULL, "No font to destroy");

    Hashmap_destroy(font->char_textures,
                    (Hashmap_destroy_func)GfxFontChar_destroy);
    FT_Done_Face(font->face);
    free(font->name);
    free(font);

    return;
error:
    return;
}

GfxFontChar *GfxFont_get_char(GfxFont *font, char c, int stroke,
                              Graphics *graphics) {
    check(font != NULL, "No font to get char in");

    char str[12] = {c};
    sprintf(str + 1, "%010d", stroke);
    str[11] = '\0';
    bstring bstr = bfromcstr(str);
    GfxFontChar *fontchar = Hashmap_get(font->char_textures, bstr);
    if (fontchar) {
        free(bstr);
        return fontchar;
    }

    int rc = FT_Load_Char(font->face, (long unsigned)c, FT_LOAD_RENDER);
    check(rc == 0, "Could not load char %c", c);

    FT_GlyphSlot g = font->face->glyph;
    FT_Glyph glyph;
    FT_Stroker  stroker = NULL;
    if (stroke) {
      FT_Stroker_New(graphics->ft, &stroker);
      FT_Stroker_Set(stroker, stroke, FT_STROKER_LINECAP_ROUND,
                     FT_STROKER_LINEJOIN_ROUND, 0);
      FT_Get_Glyph(g, &glyph);
      FT_Glyph_Stroke(&glyph, stroker, 1);
      FT_Glyph_To_Bitmap(&glyph, ft_render_mode_normal, NULL, 1);
      FT_BitmapGlyph bit_glyph = (FT_BitmapGlyph)glyph;

      fontchar = GfxFontChar_create(&bit_glyph->bitmap, g->advance, g->bitmap_top);

      FT_Done_Glyph(glyph);
      FT_Stroker_Done(stroker);
    } else {
      FT_Get_Glyph(g, &glyph);
      fontchar = GfxFontChar_create(&g->bitmap, g->advance, g->bitmap_top);
      FT_Done_Glyph(glyph);
    }

    if (fontchar) {
      Hashmap_set(font->char_textures, bstr, fontchar);
    } else {
      bdestroy(bstr);
    }

    return fontchar;
error:
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////

#pragma mark - GfxShader

void GfxShader_destroy(GfxShader *shader, Graphics *graphics) {
  check(shader != NULL, "No shader to destroy");

  graphics->del_vao(1, &shader->gl_vertex_array);
  glDeleteProgram(shader->gl_program);
  if (shader->draw_buffer) {
      DrawBuffer_destroy(shader->draw_buffer);
  }

  free(shader);

  return;
error:
  return;
}

////////////////////////////////////////////////////////////////////////////////

#pragma mark - Drawing Commands

void Graphics_stroke_path(Graphics *graphics, VPoint *points, int num_points,
                          VPoint center, GLfloat color[4], double line_width, double rotation,
                          int loop) {
    Graphics_reset_modelview_matrix(graphics);

    Graphics_translate_modelview_matrix(graphics, center.x, center.y, 0.f);
    Graphics_rotate_modelview_matrix(graphics, rotation, 0, 0, 1);

    VVector4 tex = {.raw = {0,0,0,0}};

    Graphics_uniformMatrix4fv(graphics, UNIFORM_DECAL_PROJECTION_MATRIX,
                              graphics->projection_matrix.gl,
                              GL_FALSE);

    VVector4 cVertex = {.raw = {color[0], color[1], color[2], color[3]}};
    VVector4 aVertex = {.raw = {1, 1, 1, 1}};

    // Transpose modelview matrix because attribute reads columns, not rows.
    VMatrix tmvm = graphics->modelview_matrix;

    VVector4 vertices[8 * num_points];
    int i = 0;
    for (i = 0; i < num_points; i++) {
        int pos_idx   = i * 8;
        int color_idx = i * 8 + 1;
        int alpha_idx = i * 8 + 2;
        int tex_idx   = i * 8 + 3;
        int mvm_1_idx = i * 8 + 4;
        int mvm_2_idx = i * 8 + 5;
        int mvm_3_idx = i * 8 + 6;
        int mvm_4_idx = i * 8 + 7;

        VPoint point = points[i];
        VVector4 pos = {.raw = {point.x, point.y, 0.0, 1.0}};
        vertices[pos_idx] = pos;
        vertices[color_idx] = cVertex;
        vertices[alpha_idx] = aVertex;
        vertices[tex_idx] = tex;
        vertices[mvm_1_idx] = tmvm.v[0];
        vertices[mvm_2_idx] = tmvm.v[1];
        vertices[mvm_3_idx] = tmvm.v[2];
        vertices[mvm_4_idx] = tmvm.v[3];
    }
    glBufferData(GL_ARRAY_BUFFER, 8 * num_points * sizeof(VVector4), vertices,
            GL_STATIC_DRAW);

    Graphics_uniform1f(graphics,
                       UNIFORM_DECAL_TEX_ALPHA_ADJ,
                       0.0);
#ifdef DABES_SDL
    glDisable(GL_MULTISAMPLE);
#endif
    glLineWidth(line_width);
    glDrawArrays(loop ? GL_LINE_LOOP : GL_LINE_STRIP, 0, num_points);
    return;
#ifdef DABES_SDL
    glEnable(GL_MULTISAMPLE);
#endif
}

void Graphics_stroke_circle(Graphics *graphics, VCircle circle, int precision,
        VPoint center, GLfloat color[4], double line_width) {
    VPoint points[precision];
    check(graphics != NULL, "Graphics required");
    
    if (circle.radius <= 0) {
        return;
    }

    int i = 0;
    for (i = 0; i < precision; i++) {
        float angle_rad = (1.f * i / precision) * M_PI * 2;
        VPoint point = VPoint_make(
            cosf(angle_rad) * circle.radius,
            sinf(angle_rad) * circle.radius
        );
        points[i] = point;
    }

    Graphics_stroke_path(graphics, points, precision,
                         VPoint_add(center, circle.center),
                         color, line_width, 0, 1);

    return;
error:
    return;
}

void Graphics_stroke_rect(Graphics *graphics, VRect rect, GLfloat color[4],
                          double line_width, double rotation) {
    double w = rect.tr.x - rect.tl.x;
    double h = rect.bl.y - rect.tl.y;
    VPoint center = {
        rect.tl.x + w / 2,
        rect.tl.y + h / 2
    };

    int i = 0;
    VPoint poly[4];
    for (i = 0; i < 4; i ++) {
        poly[i] = VPoint_subtract(VRect_vertex(rect, i), center);
    }

    Graphics_stroke_path(graphics, poly, 4, center, color, line_width, rotation, 1);
}


void Graphics_draw_sprite(Graphics *graphics, struct Sprite *sprite,
                          struct DrawBuffer *draw_buffer, VRect rect,
                          GLfloat color[4], double rot_degs, int z_index,
                          GLfloat alpha) {
    SpriteFrame *frame = &sprite->frames[sprite->current_frame];
    VPoint frame_offset = frame->offset;
    frame_offset.x += sprite->padding;
    frame_offset.y += sprite->padding;
    GfxSize draw_size = sprite->cell_size;

    if (sprite->direction == SPRITE_DIR_FACING_LEFT) {
        frame_offset.x += sprite->cell_size.w;
        draw_size.w = -sprite->cell_size.w;
    }
    Graphics_draw_rect(graphics, draw_buffer, rect, color, sprite->texture,
                       frame_offset, draw_size, rot_degs, z_index, alpha);
}

void Graphics_draw_rect(Graphics *graphics, struct DrawBuffer *draw_buffer,
        VRect rect, GLfloat color[4], GfxTexture *texture, VPoint textureOffset,
        GfxSize textureSize, double rotation, int z_index, GLfloat alpha) {
    check_mem(graphics);
    Graphics_reset_modelview_matrix(graphics);
    double w = rect.tr.x - rect.tl.x;
    double h = rect.bl.y - rect.tl.y;

    VPoint center = {
        rect.tl.x + w / 2,
        rect.tl.y + h / 2
    };
    Graphics_translate_modelview_matrix(graphics, center.x, center.y, 0.f);
    Graphics_rotate_modelview_matrix(graphics, rotation, 0, 0, 1);

    VRect tex_rect = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
    VPoint pot_scale = {1, 1};
    GfxSize texel_size = {1, 1};
    if (texture) {

        pot_scale.x = texture->size.w / texture->pot_size.w;
        pot_scale.y = texture->size.h / texture->pot_size.h;

        textureOffset.x /= texture->size.w;
        textureOffset.y /= texture->size.h;

        textureSize.w /= texture->size.w;
        textureSize.h /= texture->size.h;

        VPoint size_vec = {textureSize.w, textureSize.h};
        int i = 0;
        for (i = 0; i < 4; i++) {
            VPoint vertex = VRect_vertex(tex_rect, i);
            vertex = VPoint_multiply(vertex, size_vec);
            vertex = VPoint_add(vertex, textureOffset);
            vertex = VPoint_multiply(vertex, pot_scale);
            VRect_set_vertex(&tex_rect, i, vertex);
        }

        texel_size.w = 1 / texture->pot_size.w;
        texel_size.h = 1 / texture->pot_size.h;
    }

    VVector4 cVertex = {.raw = {color[0], color[1], color[2], color[3]}};
    VVector4 aVertex = {.raw = {1, 1, 1, alpha}};

    // Transpose modelview matrix because attribute reads columns, not rows.
    VMatrix tmvm = graphics->modelview_matrix;

    VVector4 vertices[8 * 6] = {
        {.raw = {-w / 2.0, -h / 2.0, 0, 1}},
            cVertex,
            aVertex,
            {.raw={tex_rect.tl.x, tex_rect.tl.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],
        {.raw = {w / 2.0, -h / 2.0, 0, 1}},
            cVertex,
            aVertex,
            {.raw={tex_rect.tr.x, tex_rect.tr.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],
        {.raw = {w / 2.0, h / 2.0, 0, 1}},
            cVertex,
            aVertex,
            {.raw={tex_rect.br.x,tex_rect.br.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],

        {.raw = {w / 2.0, h / 2.0, 0, 1}},
            cVertex,
            aVertex,
            {.raw={tex_rect.br.x,tex_rect.br.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],
        {.raw = {-w / 2.0, h / 2.0, 0, 1}},
            cVertex,
            aVertex,
            {.raw={tex_rect.bl.x, tex_rect.bl.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],
        {.raw = {-w / 2.0, -h / 2.0, 0, 1}},
            cVertex,
            aVertex,
            {.raw={tex_rect.tl.x, tex_rect.tl.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],
    };

    if (draw_buffer) {
        DrawBuffer_buffer(draw_buffer, texture, z_index, 6, 8, vertices);
    } else {
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        if (texture) {
          Graphics_uniform1f(graphics,
                             UNIFORM_DECAL_TEX_ALPHA_ADJ,
                             1.0);
          glBindTexture(GL_TEXTURE_2D, texture->gl_tex);
        } else {
          Graphics_uniform1f(graphics,
                             UNIFORM_DECAL_TEX_ALPHA_ADJ,
                             0.0);
          glBindTexture(GL_TEXTURE_2D, 0);
        }
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    return;
error:
    return;
}

void Graphics_draw_string(Graphics *graphics, char *text, GfxFont *font,
        GLfloat color[4], VPoint origin, GfxTextAlign align,
        GLfloat shadow_color[4], VPoint shadow_offset) {

    if (shadow_color[3] > 0.0 &&
            VPoint_rel(shadow_offset, VPointZero) != VPointRelWithin) {
        // This is totally lazy and could be optimized.
        VPoint so = VPoint_add(origin, shadow_offset);
        Graphics_draw_string(graphics, text, font, shadow_color, so,
                             align, GfxGLClearColor, VPointZero);
    }

    char *c = text;
    Graphics_uniformMatrix4fv(graphics,
                              UNIFORM_TEXT_PROJECTION_MATRIX,
                              graphics->projection_matrix.gl,
                              GL_FALSE);
  
    VVector4 cVertex = {.raw = {color[0], color[1], color[2], color[3]}};
    Graphics_uniform4f(graphics,
                       UNIFORM_TEXT_COLOR,
                       cVertex.r, cVertex.g, cVertex.b, cVertex.a);
  

    // Unfortunately we need to do a pre-pass for right and center.
    float line_width = 0;
    if (align != GfxTextAlignLeft) {
      while (*c != '\0') {
          GfxFontChar *fontchar = GfxFont_get_char(font, *c, 0, graphics);
          if (fontchar == NULL) {
            c++;
            continue;
          }

          line_width += fontchar->advance.x / 64.0;
          c++;
      }
      line_width = floorf(line_width);
    }

    c = text;
    float xo = 0;
    while (*c != '\0') {
        GfxFontChar *fontchar = GfxFont_get_char(font, *c, 0, graphics);
        if (fontchar == NULL) {
          c++;
          continue;
        }

        GfxTexture *texture = fontchar->texture;

        VPoint pot_scale = {1, 1};
        if (texture) {
            pot_scale.x = texture->size.w / texture->pot_size.w;
            pot_scale.y = texture->size.h / texture->pot_size.h;
        }
        Graphics_uniform2f(graphics,
                           UNIFORM_TEXT_STRETCH,
                           pot_scale.x, pot_scale.y);
      
        VPoint trans = VPointZero;
        switch (align) {
          case GfxTextAlignRight: {
            trans = VPoint_make(origin.x - line_width + xo,
                                origin.y - fontchar->bitmap_top);
          } break;

          case GfxTextAlignCenter: {
            trans = VPoint_make(origin.x - line_width / 2 + xo,
                                origin.y - fontchar->bitmap_top);
          } break;

          case GfxTextAlignLeft:
          default: {
            trans = VPoint_make(origin.x + xo,
                                origin.y - fontchar->bitmap_top);
          } break;

        }
      
        Graphics_reset_modelview_matrix(graphics);
        Graphics_translate_modelview_matrix(graphics, trans.x, trans.y, 0);
        Graphics_scale_modelview_matrix(graphics,
                                        texture->size.w, texture->size.h, 1);
        Graphics_uniformMatrix4fv(graphics,
                                  UNIFORM_TEXT_MODELVIEW_MATRIX,
                                  graphics->modelview_matrix.gl, GL_FALSE);

        Graphics_uniform1i(graphics, UNIFORM_TEXT_TEXTURE, 0);
        glBindTexture(GL_TEXTURE_2D, texture->gl_tex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        xo += fontchar->advance.x / 64.0;
        c++;
    }

    return;
}

#pragma mark - Uniforms

void Graphics_get_uniform(Graphics *graphics, GLint program, const GLchar *name,
                          GraphicsUniform uniform) {
  GLint loc = glGetUniformLocation(program, name);
  GfxShader_uniforms[uniform] = loc;
  graphics->num_uniforms = MAX(graphics->num_uniforms, loc + 1);
}

void Graphics_clear_uniforms(Graphics *graphics) {
  int i = 0;
  for (i = 0; i < graphics->num_uniforms; i++) {
    if (graphics->uniforms[i] != NULL) {
      free(graphics->uniforms[i]);
      graphics->uniforms[i] = NULL;
    }
  }
}

void Graphics_uniformMatrix4fv(Graphics *graphics, GraphicsUniform uniform,
                               GLfloat vector[16], GLboolean transpose) {
  int loc = GfxShader_uniforms[uniform];
  if (loc < 0) return;
  
  if (graphics->uniforms[loc]) {
     int cmp = memcmp(vector, graphics->uniforms[loc], sizeof(GLfloat) * 16);
     if (cmp == 0) {
       return;
     }
  }
  
  GLfloat *buf = realloc(graphics->uniforms[loc], 16 * sizeof(GLfloat));
  memcpy(buf, vector, sizeof(GLfloat) * 16);
  graphics->uniforms[loc] = buf;
  glUniformMatrix4fv(loc, 1, transpose, vector);
}

void Graphics_uniform1i(Graphics *graphics, GraphicsUniform uniform,
                        GLint x) {
  int loc = GfxShader_uniforms[uniform];
  if (loc < 0) return;
  
  if (graphics->uniforms[loc]) {
    int cmp = memcmp(&x, graphics->uniforms[loc], sizeof(GLint));
    if (cmp == 0) {
      return;
    }
  }
  
  GLint *buf = realloc(graphics->uniforms[loc], sizeof(GLint));
  memcpy(buf, &x, sizeof(GLint));
  graphics->uniforms[loc] = buf;
  glUniform1i(loc, x);
}

void Graphics_uniform1f(Graphics *graphics, GraphicsUniform uniform,
                        GLfloat x) {
  int loc = GfxShader_uniforms[uniform];
  if (loc < 0) return;
  
  if (graphics->uniforms[loc]) {
    int cmp = memcmp(&x, graphics->uniforms[loc], sizeof(GLfloat));
    if (cmp == 0) {
      return;
    }
  }
  
  GLfloat *buf = realloc(graphics->uniforms[loc], sizeof(GLfloat));
  memcpy(buf, &x, sizeof(GLfloat));
  graphics->uniforms[loc] = buf;
  glUniform1f(loc, x);
}

void Graphics_uniform2f(Graphics *graphics, GraphicsUniform uniform,
                        GLfloat x, GLfloat y) {
  int loc = GfxShader_uniforms[uniform];
  if (loc < 0) return;
  
  GLfloat vector[2] = {x, y};
  if (graphics->uniforms[loc]) {
    int cmp = memcmp(vector, graphics->uniforms[loc], sizeof(GLfloat) * 2);
    if (cmp == 0) {
      return;
    }
  }
  
  GLfloat *buf = realloc(graphics->uniforms[loc], 2 * sizeof(GLfloat));
  memcpy(buf, vector, sizeof(GLfloat) * 2);
  graphics->uniforms[loc] = buf;
  glUniform2f(loc, x, y);
}

void Graphics_uniform4f(Graphics *graphics, GraphicsUniform uniform,
                        GLfloat x, GLfloat y, GLfloat z, GLfloat w) {
  int loc = GfxShader_uniforms[uniform];
  if (loc < 0) return;
  
  GLfloat vector[4] = {x, y, z, w};
  if (graphics->uniforms[loc]) {
    int cmp = memcmp(vector, graphics->uniforms[loc], sizeof(GLfloat) * 4);
    if (cmp == 0) {
      return;
    }
  }
  
  GLfloat *buf = realloc(graphics->uniforms[loc], 4 * sizeof(GLfloat));
  memcpy(buf, vector, sizeof(GLfloat) * 4);
  graphics->uniforms[loc] = buf;
  glUniform4f(loc, x, y, z, w);
}

#pragma mark - Matrices

void Graphics_reset_projection_matrix(Graphics *graphics) {
    graphics->projection_matrix = VMatrixIdentity;
}

void Graphics_ortho_projection_matrix(Graphics *graphics, double left,
        double right, double top, double bottom, double near, double far) {
    graphics->projection_matrix =
        VMatrix_make_ortho(left, right, top, bottom, near, far);
}

void Graphics_perspective_projection_matrix(Graphics *graphics,
                                            float fov_radians, float aspect,
                                            float near, float far) {
    graphics->projection_matrix =
        VMatrix_make_perspective(fov_radians, aspect, near, far);
}

void Graphics_frustum_projection_matrix(Graphics *graphics, double left,
        double right, double top, double bottom, double near, double far) {
    graphics->projection_matrix =
        VMatrix_make_frustum(left, right, top, bottom, near, far);
}

void Graphics_scale_projection_matrix(Graphics *graphics, double x,
        double y, double z) {
    graphics->projection_matrix =
        VMatrix_scale(graphics->projection_matrix, x, y, z);
}

void Graphics_rotate_projection_matrix(Graphics *graphics, double rot_degs,
                                       double x, double y, double z) {
    graphics->projection_matrix =
        VMatrix_rotate(graphics->projection_matrix, rot_degs, x, y, z);
}

void Graphics_translate_projection_matrix(Graphics *graphics, double x,
        double y, double z) {
    graphics->projection_matrix =
        VMatrix_translate(graphics->projection_matrix, x, y, z);
}

void Graphics_reset_modelview_matrix(Graphics *graphics) {
    graphics->modelview_matrix = VMatrixIdentity;
}

void Graphics_scale_modelview_matrix(Graphics *graphics,
                                     double x, double y, double z) {
    graphics->modelview_matrix =
        VMatrix_scale(graphics->modelview_matrix, x, y, z);
}

void Graphics_rotate_modelview_matrix(Graphics *graphics, double rot_degs,
                                      double x, double y, double z) {
    graphics->modelview_matrix =
        VMatrix_rotate(graphics->modelview_matrix, rot_degs, x, y, z);
}

void Graphics_translate_modelview_matrix(Graphics *graphics,
                                         double x, double y, double z) {
    graphics->modelview_matrix =
        VMatrix_translate(graphics->modelview_matrix, x, y, z);
}

#pragma mark - Decal Shader

void set_up_decal_shader(GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_VERTEX]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_COLOR]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_ALPHA]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_TEXTURE]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 1);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 2);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 3);

    // Position, color, texture, modelView * 4
    int v_size = sizeof(VVector4) * 8;

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_VERTEX], 4,
                          GL_FLOAT, GL_FALSE, v_size, 0);

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_COLOR], 4,
                          GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 1));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_ALPHA], 4,
                          GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 2));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_TEXTURE], 4,
                          GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 3));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX],
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 4));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 1,
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 5));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 2,
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 6));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 3,
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 7));

    graphics->bind_vao(0);
}

void tear_down_decal_shader(GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_VERTEX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_COLOR]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_ALPHA]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_TEXTURE]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 1);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 2);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 3);

    graphics->bind_vao(0);
}

void Graphics_build_decal_shader(Graphics *graphics) {
    GfxShader *shader = calloc(1, sizeof(GfxShader));
    check(shader != NULL, "Could not alloc decal shader");

    int rc = Graphics_load_shader(graphics, shader_path("decal.vert"),
        shader_path("decal.frag"), &shader->gl_program);
    assert(rc == 1);
    Hashmap_set(graphics->shaders, bfromcstr("decal"), shader);
    List_push(graphics->shader_list, shader);

    GLuint program = shader->gl_program;
    Graphics_get_uniform(graphics, program,
                         "projection",
                          UNIFORM_DECAL_PROJECTION_MATRIX);
    Graphics_get_uniform(graphics, program,
                         "texAlphaAdj",
                         UNIFORM_DECAL_TEX_ALPHA_ADJ);
    GfxShader_attributes[ATTRIB_DECAL_VERTEX] =
        glGetAttribLocation(program, "position");
    GfxShader_attributes[ATTRIB_DECAL_COLOR] =
        glGetAttribLocation(program, "color");
    GfxShader_attributes[ATTRIB_DECAL_ALPHA] =
        glGetAttribLocation(program, "alpha");
    GfxShader_attributes[ATTRIB_DECAL_TEXTURE] =
        glGetAttribLocation(program, "texture");
    GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] =
        glGetAttribLocation(program, "modelView");

    graphics->gen_vao(1, &shader->gl_vertex_array);

    set_up_decal_shader(shader, graphics);
    shader->set_up = &set_up_decal_shader;
    shader->tear_down = &tear_down_decal_shader;
  
    shader->gl_vertex_buffer = graphics->array_buffer;

    shader->draw_buffer = DrawBuffer_create();
    return;
error:
    if (shader) free(shader);
    return;
}

#pragma mark - Tilemap Shader

void set_up_tilemap_shader(GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TILEMAP_VERTEX]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TILEMAP_TEXTURE]);
    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TILEMAP_VERTEX], 4,
                          GL_FLOAT, GL_FALSE, 0, 0);

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TILEMAP_TEXTURE], 4,
                          GL_FLOAT, GL_FALSE, 0,
                          (GLvoid *)(sizeof(VVector4) * 4));

    graphics->bind_vao(0);
}

void tear_down_tilemap_shader (GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TILEMAP_VERTEX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TILEMAP_TEXTURE]);

    graphics->bind_vao(0);
}

void Graphics_build_tilemap_shader(Graphics *graphics) {
    GfxShader *shader = calloc(1, sizeof(GfxShader));
    check(shader != NULL, "Could not alloc tilemap shader");

    int rc = Graphics_load_shader(graphics, shader_path("tilemap.vert"),
        shader_path("tilemap.frag"), &shader->gl_program);
    assert(rc == 1);
    Hashmap_set(graphics->shaders, bfromcstr("tilemap"), shader);
    List_push(graphics->shader_list, shader);

    GLuint program = shader->gl_program;
    Graphics_get_uniform(graphics, program,
                         "modelView",
                         UNIFORM_TILEMAP_MODELVIEW_MATRIX);
    Graphics_get_uniform(graphics, program,
                         "projection",
                         UNIFORM_TILEMAP_PROJECTION_MATRIX);
    Graphics_get_uniform(graphics, program,
                         "tileSize",
                         UNIFORM_TILEMAP_TILE_SIZE);
    Graphics_get_uniform(graphics, program,
                         "sheetRowsCols",
                         UNIFORM_TILEMAP_SHEET_ROWS_COLS);
    Graphics_get_uniform(graphics, program,
                         "sheetPotSize",
                         UNIFORM_TILEMAP_SHEET_POT_SIZE);
    Graphics_get_uniform(graphics, program,
                         "sheetPortion",
                         UNIFORM_TILEMAP_SHEET_PORTION);
    Graphics_get_uniform(graphics, program,
                         "mapRowsCols",
                         UNIFORM_TILEMAP_MAP_ROWS_COLS);
    Graphics_get_uniform(graphics, program,
                         "texelPerMap",
                         UNIFORM_TILEMAP_TEXEL_PER_MAP);
    Graphics_get_uniform(graphics, program,
                         "atlas",
                         UNIFORM_TILEMAP_ATLAS);
    Graphics_get_uniform(graphics, program,
                         "tileset",
                         UNIFORM_TILEMAP_TILESET);
  
    GfxShader_attributes[ATTRIB_TILEMAP_VERTEX] =
        glGetAttribLocation(program, "position");
    GfxShader_attributes[ATTRIB_TILEMAP_TEXTURE] =
        glGetAttribLocation(program, "texture");

    graphics->gen_vao(1, &shader->gl_vertex_array);

    set_up_tilemap_shader(shader, graphics);
    shader->set_up = &set_up_tilemap_shader;
    shader->tear_down = &tear_down_tilemap_shader;
  
    shader->gl_vertex_buffer = graphics->array_buffer;
  
    return;
error:
    if (shader) free(shader);
    return;
}

#pragma mark - Parallax Shader

void set_up_parallax_shader(GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_PARALLAX_VERTEX]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_PARALLAX_TEXTURE]);
  
    glBindBuffer(GL_ARRAY_BUFFER, shader->gl_vertex_buffer);
  
    glVertexAttribPointer(GfxShader_attributes[ATTRIB_PARALLAX_VERTEX], 4,
                          GL_FLOAT, GL_FALSE, 0, 0);

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_PARALLAX_TEXTURE], 4,
                          GL_FLOAT, GL_FALSE, 0,
                          (GLvoid *)(sizeof(VVector4) * 4));
  
    glBindBuffer(GL_ARRAY_BUFFER, graphics->array_buffer);

    graphics->bind_vao(0);
}

void tear_down_parallax_shader (GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_PARALLAX_VERTEX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_PARALLAX_TEXTURE]);
    graphics->bind_vao(0);
  
    glBindBuffer(GL_ARRAY_BUFFER, graphics->array_buffer);
}

void Graphics_build_parallax_shader(Graphics *graphics) {
    GfxShader *shader = calloc(1, sizeof(GfxShader));
    check(shader != NULL, "Could not alloc parallax shader");

    int rc = Graphics_load_shader(graphics, shader_path("parallax.vert"),
        shader_path("parallax.frag"), &shader->gl_program);
    assert(rc == 1);
    Hashmap_set(graphics->shaders, bfromcstr("parallax"), shader);
    List_push(graphics->shader_list, shader);

    GLuint program = shader->gl_program;
    Graphics_get_uniform(graphics, program,
                         "modelView",
                         UNIFORM_PARALLAX_MODELVIEW_MATRIX);
    Graphics_get_uniform(graphics, program,
                         "projection",
                         UNIFORM_PARALLAX_PROJECTION_MATRIX);
    Graphics_get_uniform(graphics, program,
                         "texture",
                         UNIFORM_PARALLAX_TEXTURE);
    Graphics_get_uniform(graphics, program,
                         "texPortion",
                         UNIFORM_PARALLAX_TEX_PORTION);
    Graphics_get_uniform(graphics, program,
                         "stretch",
                          UNIFORM_PARALLAX_STRETCH);
    Graphics_get_uniform(graphics, program,
                         "cascadeTop",
                         UNIFORM_PARALLAX_CASCADE_TOP);
    Graphics_get_uniform(graphics, program,
                         "cascadeBottom",
                         UNIFORM_PARALLAX_CASCADE_BOTTOM);
    Graphics_get_uniform(graphics, program,
                         "origPixel",
                         UNIFORM_PARALLAX_ORIG_PIXEL);
    Graphics_get_uniform(graphics, program,
                         "repeatSize",
                         UNIFORM_PARALLAX_REPEAT_SIZE);
    Graphics_get_uniform(graphics, program,
                         "repeats",
                         UNIFORM_PARALLAX_REPEATS);
    Graphics_get_uniform(graphics, program,
                         "xShift",
                         UNIFORM_PARALLAX_X_SHIFT);
    Graphics_get_uniform(graphics, program,
                         "cameraPos",
                         UNIFORM_PARALLAX_CAMERA_POS);
    Graphics_get_uniform(graphics, program,
                         "parallaxFactor",
                          UNIFORM_PARALLAX_FACTOR);
    Graphics_get_uniform(graphics, program,
                         "texScale",
                          UNIFORM_PARALLAX_TEX_SCALE);
    GfxShader_attributes[ATTRIB_PARALLAX_VERTEX] =
        glGetAttribLocation(program, "position");
    GfxShader_attributes[ATTRIB_PARALLAX_TEXTURE] =
        glGetAttribLocation(program, "texture");
  
    glGenBuffers(1, &shader->gl_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shader->gl_vertex_buffer);
    VVector4 texpos[4] = {
      {.raw = {0, 0, 0, 1}},
      {.raw = {1.0, 0, 0, 1}},
      {.raw = {0, 1.0, 0, 1}},
      {.raw = {1.0, 1.0, 0, 1}}
    };

    VRect rect = VRect_from_xywh(0, 0, 1, 1);
    VVector4 vertices[8] = {
          // Vertex
          {.raw = {rect.tl.x, rect.tl.y, 0, 1}},
          {.raw = {rect.tr.x, rect.tr.y, 0, 1}},
          {.raw = {rect.bl.x, rect.bl.y, 0, 1}},
          {.raw = {rect.br.x, rect.br.y, 0, 1}},

          // Texture
          texpos[0], texpos[1], texpos[2], texpos[3]
    };
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(VVector4), vertices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->array_buffer);
  
    graphics->gen_vao(1, &shader->gl_vertex_array);

    set_up_parallax_shader(shader, graphics);
    shader->set_up = &set_up_parallax_shader;
    shader->tear_down = &tear_down_parallax_shader;
    return;
error:
    if (shader) free(shader);
    return;
}

#pragma mark - Text Shader

void set_up_text_shader(GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_VERTEX]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_TEX_POS]);

    glBindBuffer(GL_ARRAY_BUFFER, shader->gl_vertex_buffer);
  
    int v_size = sizeof(VVector4) * 2;

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TEXT_VERTEX], 4,
                          GL_FLOAT, GL_FALSE, v_size, 0);

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TEXT_TEX_POS], 4,
                          GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 1));

    glBindBuffer(GL_ARRAY_BUFFER, graphics->array_buffer);

    graphics->bind_vao(0);
}

void tear_down_text_shader(GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_VERTEX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_TEX_POS]);

    graphics->bind_vao(0);
}

void Graphics_build_text_shader(Graphics *graphics) {
    GfxShader *shader = calloc(1, sizeof(GfxShader));
    check(shader != NULL, "Could not alloc text shader");

    int rc = Graphics_load_shader(graphics, shader_path("text.vert"),
        shader_path("text.frag"), &shader->gl_program);
    assert(rc == 1);
    Hashmap_set(graphics->shaders, bfromcstr("text"), shader);
    List_push(graphics->shader_list, shader);

    GLuint program = shader->gl_program;
    Graphics_get_uniform(graphics, program,
                         "texture",
                         UNIFORM_TEXT_TEXTURE);
    Graphics_get_uniform(graphics, program,
                         "projection",
                         UNIFORM_TEXT_PROJECTION_MATRIX);
    Graphics_get_uniform(graphics, program,
                         "modelView",
                         UNIFORM_TEXT_MODELVIEW_MATRIX);
    Graphics_get_uniform(graphics, program,
                         "stretch",
                         UNIFORM_TEXT_STRETCH);
    Graphics_get_uniform(graphics, program,
                         "color",
                         UNIFORM_TEXT_COLOR);
    GfxShader_attributes[ATTRIB_TEXT_VERTEX] =
        glGetAttribLocation(program, "position");
    GfxShader_attributes[ATTRIB_TEXT_TEX_POS] =
        glGetAttribLocation(program, "texPos");

    glGenBuffers(1, &shader->gl_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, shader->gl_vertex_buffer);
    VVector4 texpos[4] = {
      {.raw = {0, 0, 0, 1}},
      {.raw = {1.0, 0, 0, 1}},
      {.raw = {0, 1.0, 0, 1}},
      {.raw = {1.0, 1.0, 0, 1}}
    };

    VRect rect = VRect_from_xywh(0, 0, 1, 1);
    VVector4 vertices[2 * 6] = {
          {.raw = {rect.tl.x, rect.tl.y, 0, 1}},
          texpos[0],
      
          {.raw = {rect.tr.x, rect.tr.y, 0, 1}},
          texpos[1],
      
          {.raw = {rect.br.x, rect.br.y, 0, 1}},
          texpos[3],
           
          {.raw = {rect.br.x, rect.br.y, 0, 1}},
          texpos[3],
      
          {.raw = {rect.bl.x, rect.bl.y, 0, 1}},
          texpos[2],
      
          {.raw = {rect.tl.x, rect.tl.y, 0, 1}},
          texpos[0],
    };
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(VVector4), vertices,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->array_buffer);
  
    graphics->gen_vao(1, &shader->gl_vertex_array);

    set_up_text_shader(shader, graphics);
    shader->set_up = &set_up_text_shader;
    shader->tear_down = &tear_down_text_shader;

    shader->draw_buffer = DrawBuffer_create();
    return;
error:
    if (shader) free(shader);
    return;
}

#pragma mark - Graphics

Graphics *Graphics_create(Engine *engine) {
    Graphics *graphics = calloc(1, sizeof(Graphics));
    check(graphics != NULL, "Couldn't create graphics");

    graphics->engine = engine;

    graphics->screen_size.w = SCREEN_WIDTH;
    graphics->screen_size.h = SCREEN_HEIGHT;
  
    glGenBuffers(1, &graphics->array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->array_buffer);
  
    glClearColor(0.65f, 0.65f, 0.65f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    graphics->current_shader = NULL;
    graphics->shaders = Hashmap_create(NULL, Hashmap_fnv1a_hash);
    graphics->shader_list = List_create();

    graphics->gl_vao_enabled = 0;
#ifdef DABES_IOS
    graphics->gl_vao_enabled = 1;
    graphics->gen_vao = glGenVertexArraysOES;
    graphics->bind_vao = glBindVertexArrayOES;
    graphics->del_vao = glDeleteVertexArraysOES;
#endif
#ifdef DABES_MAC
    graphics->gl_vao_enabled = 1;
    graphics->gen_vao = glGenVertexArraysAPPLE;
    graphics->bind_vao = glBindVertexArrayAPPLE;
    graphics->del_vao = glDeleteVertexArraysAPPLE;
#endif
#ifdef DABES_SDL
    if (strstr((char *)glGetString(GL_EXTENSIONS),
               "GL_ARB_vertex_array_object")) {
        graphics->gl_vao_enabled = 1;
        graphics->gen_vao = SDL_GL_GetProcAddress("glGenVertexArraysARB");
        graphics->bind_vao = SDL_GL_GetProcAddress("glBindVertexArrayARB");
        graphics->del_vao = SDL_GL_GetProcAddress("glDeleteVertexArraysARB");
    } else if (strstr((char *)glGetString(GL_EXTENSIONS),
                      "GL_APPLE_vertex_array_object")) {
        graphics->gl_vao_enabled = 1;
        graphics->gen_vao = SDL_GL_GetProcAddress("glGenVertexArraysAPPLE");
        graphics->bind_vao = SDL_GL_GetProcAddress("glBindVertexArrayAPPLE");
        graphics->del_vao = SDL_GL_GetProcAddress("glDeleteVertexArraysAPPLE");
    }
#endif

    // graphics->num_uniforms is still 0 here. It gets populated by the shader
    // compiles.
  
    Graphics_build_decal_shader(graphics);
    Graphics_build_tilemap_shader(graphics);
    Graphics_build_parallax_shader(graphics);
    Graphics_build_text_shader(graphics);
  
    // Now we have the uniform count
    graphics->uniforms = calloc(graphics->num_uniforms, sizeof(void *));

    FT_Library ft = NULL;
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "Could not init freetype library\n");
    }
    graphics->ft = ft;

    graphics->textures = Hashmap_create(NULL, Hashmap_fnv1a_hash);
    graphics->sprites = Hashmap_create(NULL, Hashmap_fnv1a_hash);

    char *fontpath = engine->resource_path("fonts/uni.ttf");
    graphics->debug_font = GfxFont_create(graphics, fontpath, 16);
    free(fontpath);

    return graphics;
error:
    return NULL;
}

void nulldestroy(void *UNUSED(obj)) { }

void Graphics_destroy(Graphics *graphics) {
    GfxFont_destroy(graphics->debug_font);
    Hashmap_destroy(graphics->shaders, nulldestroy);
    LIST_FOREACH(graphics->shader_list, first, next, current) {
        GfxShader *shader = current->value;
        GfxShader_destroy(shader, graphics);
    }
    List_destroy(graphics->shader_list);

    Hashmap_destroy(graphics->textures,
                    (Hashmap_destroy_func)GfxTexture_destroy);

    Hashmap_destroy(graphics->sprites,
                    (Hashmap_destroy_func)Sprite_destroy);

    free(graphics);
}

GLuint Graphics_load_shader(Graphics *graphics, char *vert_name,
        char *frag_name, GLuint *compiled_program) {
    GLuint vertex_shader, fragment_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    check(vertex_shader != 0, "Couldn't create vertex shader");
    check(fragment_shader != 0, "Couldn't create fragment shader");

    unsigned char *v_src, *f_src;
    GLint v_size, f_size;
    Engine_load_resource(graphics->engine, vert_name, &v_src, &v_size);
    Engine_load_resource(graphics->engine, frag_name, &f_src, &f_size);

    glShaderSource(vertex_shader, 1, (const GLchar**)&v_src, &v_size);
    glShaderSource(fragment_shader, 1, (const GLchar**)&f_src, &f_size);
    free(v_src);
    free(f_src);

    GLint compiled = 0;

    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    Graphics_log_shader(vertex_shader);
    check(compiled == GL_TRUE, "Vertex shader failed to compile");

    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
    Graphics_log_shader(fragment_shader);
    check(compiled == GL_TRUE, "Fragment shader failed to compile");

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    Graphics_log_program(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    check(linked == 1, "Linking shader program");

    glDetachShader(program, vertex_shader);
    glDetachShader(program, fragment_shader);
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    //if(strcmp(vert_name, shader_path("decal.vert")) == 0) return 1;
    *compiled_program = program;
    return 1;
error:
    return 0;
}

GfxShader *Graphics_get_shader(Graphics *graphics, char *name) {
    bstring key = bfromcstr(name);
    GfxShader *val = Hashmap_get(graphics->shaders, key);
    bdestroy(key);
    if (val) return val;
    return NULL;
}

void Graphics_use_shader(Graphics *graphics, GfxShader *shader) {
    if (shader == graphics->current_shader) return;
  
    Graphics_clear_uniforms(graphics);
    if (shader == NULL) {
        graphics->bind_vao(0);

        glUseProgram(0);
        glBindBuffer(GL_ARRAY_BUFFER, graphics->array_buffer);
    } else {
        graphics->bind_vao(shader->gl_vertex_array);

        glUseProgram(shader->gl_program);
        glBindBuffer(GL_ARRAY_BUFFER, shader->gl_vertex_buffer);
    }
    graphics->current_shader = shader;
}

void Graphics_log_shader(GLuint shader) {
    GLint blen = 0;
    GLsizei slen = 0;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &blen);
    if (blen > 1) {
     GLchar* compiler_log = (GLchar*)malloc(blen);
     glGetShaderInfoLog(shader, blen, &slen, compiler_log);
     debug("compiler_log:\n%s\n", compiler_log);
     free (compiler_log);
    }
}

void Graphics_log_program(GLuint program) {
    GLint blen = 0;
    GLsizei slen = 0;

    glGetProgramiv(program, GL_INFO_LOG_LENGTH , &blen);
    if (blen > 1) {
     GLchar* linker_log = (GLchar*)malloc(blen);
     glGetProgramInfoLog(program, blen, &slen, linker_log);
     debug("linker_log:\n%s\n", linker_log);
     free (linker_log);
    }
}

GfxTexture *Graphics_texture_from_image(Graphics *graphics, const char *image_name, int mipmap) {
    bstring bimage_name = bfromcstr(image_name);

    void *val = Hashmap_get(graphics->textures, bimage_name);
    if (val != NULL) {
      bdestroy(bimage_name);
      return (GfxTexture *)val;
    }

    GfxTexture *texture = GfxTexture_from_image(image_name, mipmap);
    Hashmap_set(graphics->textures, bimage_name, texture);

    return texture;
}

Sprite *Graphics_sprite_from_image(Graphics *graphics, const char *image_name,
        GfxSize cell_size, int padding) {
    GfxTexture *texture = Graphics_texture_from_image(graphics, image_name, 1);
    Sprite *sprite = Sprite_create(texture, cell_size, padding);
    check(sprite != NULL, "Couldn't create sprite");

    return sprite;
error:
    return NULL;
}
