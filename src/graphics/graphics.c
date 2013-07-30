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

int Graphics_init_GL(int UNUSED(swidth), int UNUSED(sheight)) {
    GLenum error = glGetError();
    glEnable(GL_BLEND);
    error = glGetError();

#if defined(DABES_MAC) || defined(DABES_SDL)
    glEnable(GL_MULTISAMPLE);
#endif
    glDisable(GL_DEPTH_TEST);
    error = glGetError();

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
                              int *pot_width, int *pot_height, int components) {
  int pot_w = 2;
  int pot_h = 2;
  while (pot_w < width) pot_w <<= 1;
  while (pot_h < height) pot_h <<= 1;

  int pot_d = MAX(pot_h, pot_w);

  unsigned char *old_data = *data;
  unsigned char *resized_data = NULL;
  resized_data = realloc(old_data, pot_d * pot_d * sizeof(unsigned char) * 4);
  check(resized_data != NULL, "Couldn't realloc texture for potize");

  *data = resized_data;
  unsigned char *new_data = resized_data;

  *pot_width = pot_d;
  *pot_height = pot_d;

  int last_orig_index = (width * height - 1) * components;
  int i = 0;
  for (i = last_orig_index; i >= 0; i -= components) {
    int idx = i / components;
    int old_col = idx % width;
    int old_row = idx / width;
    int new_index = (old_row * pot_d + old_col) * 4;

    new_data[new_index] = new_data[i];         // R
    if (components > 1) new_data[new_index + 1] = new_data[i + 1]; // G
    if (components > 2) new_data[new_index + 2] = new_data[i + 2]; // B
    if (components > 3) new_data[new_index + 3] = new_data[i + 3]; // A
  }

  return 1;
error:
  return 0;
}

GfxTexture *GfxTexture_from_data(unsigned char **data, int width, int height,
        GLenum source_format) {
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
    int rc = data_potize(data, width, height, &pot_width, &pot_height, num_components);
    check(rc == 1, "Could not potize data");

    texture->pot_size.w = pot_width;
    texture->pot_size.h = pot_height;

    glGenTextures(1, &texture->gl_tex);
    glBindTexture(GL_TEXTURE_2D, texture->gl_tex);
    GLenum color_format = GL_RGBA;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pot_width,
                 pot_height, 0, color_format,
                 GL_UNSIGNED_BYTE, *data);
    GLenum er = glGetError();
    if (er != GL_NO_ERROR) {
        debug("breakpoint here");
    }
    check(er == GL_NO_ERROR, "Error loading texture: %d", er);
    return texture;
error:
    if (texture) {
        glDeleteTextures(1, &texture->gl_tex);
        free(texture);
    }
    return NULL;
}

GfxTexture *GfxTexture_from_image(const char *image_name) {
    int x, y, n;
    unsigned char *data = stbi_load(image_name, &x, &y, &n, 4);
    GfxTexture *texture =
        GfxTexture_from_data(&data, x, y, GL_RGBA);
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
        GfxTexture_from_data(&buf, bitmap->width, bitmap->rows, GL_LUMINANCE);

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

void Graphics_stroke_poly(Graphics *graphics, int num_points, VPoint *points,
        VPoint center, GLfloat color[4], double line_width, double rotation) {
    Graphics_reset_modelview_matrix(graphics);

    Graphics_translate_modelview_matrix(graphics, center.x, center.y, 0.f);
    Graphics_rotate_modelview_matrix(graphics, rotation, 0, 0, 1);

    VVector4 tex = {.raw = {0,0,0,0}};

    glUniform1i(GfxShader_uniforms[UNIFORM_DECAL_HAS_TEXTURE], 0);
    glUniformMatrix4fv(GfxShader_uniforms[UNIFORM_DECAL_PROJECTION_MATRIX], 1,
                       GL_FALSE, graphics->projection_matrix.gl);

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

    // Texture
#ifdef DABES_SDL
    glDisable(GL_MULTISAMPLE);
#endif
    glLineWidth(line_width);
    glDrawArrays(GL_LINE_LOOP, 0, num_points);
    return;
#ifdef DABES_SDL
    glEnable(GL_MULTISAMPLE);
#endif
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

    Graphics_stroke_poly(graphics, 4, poly, center, color, line_width, rotation);
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
        glUniform1i(GfxShader_uniforms[UNIFORM_DECAL_HAS_TEXTURE],
                    texture ? texture->gl_tex : 0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glUniform1i(GfxShader_uniforms[UNIFORM_DECAL_TEXTURE], 0);
        if (texture) {
          glBindTexture(GL_TEXTURE_2D, texture->gl_tex);
        } else {
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
    glUniformMatrix4fv(GfxShader_uniforms[UNIFORM_TEXT_PROJECTION_MATRIX], 1,
                       GL_FALSE, graphics->projection_matrix.gl);

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

        VRect tex_rect = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
        VPoint pot_scale = {1, 1};
        if (texture) {
            pot_scale.x = texture->size.w / texture->pot_size.w;
            pot_scale.y = texture->size.h / texture->pot_size.h;

            int i = 0;
            for (i = 0; i < 4; i++) {
                VPoint vertex = VRect_vertex(tex_rect, i);
                vertex = VPoint_multiply(vertex, pot_scale);
                VRect_set_vertex(&tex_rect, i, vertex);
            }
        }

        VVector4 cVertex = {.raw = {color[0], color[1], color[2], color[3]}};

        // Transpose modelview matrix because attribute reads columns, not rows.
        VMatrix tmvm = graphics->modelview_matrix;
      
        VRect glyph_rect = VRectZero;
        switch (align) {
          case GfxTextAlignRight: {
            glyph_rect = VRect_from_xywh(origin.x - line_width + xo,
                                         origin.y - fontchar->bitmap_top,
                                         texture->size.w,
                                         texture->size.h);
          } break;
          
          case GfxTextAlignCenter: {
            glyph_rect = VRect_from_xywh(origin.x - line_width / 2 + xo,
                                         origin.y - fontchar->bitmap_top,
                                         texture->size.w,
                                         texture->size.h);
          } break;
            
          case GfxTextAlignLeft:
          default: {
            glyph_rect = VRect_from_xywh(origin.x + xo,
                                         origin.y - fontchar->bitmap_top,
                                         texture->size.w,
                                         texture->size.h);
          } break;
            
        }

        VVector4 vertices[7 * 6] = {
            {.raw = {glyph_rect.tl.x, glyph_rect.tl.y, 0, 1}},
                cVertex,
                {.raw={tex_rect.tl.x, tex_rect.tl.y, 0, 0}},
                tmvm.v[0],
                tmvm.v[1],
                tmvm.v[2],
                tmvm.v[3],
            {.raw = {glyph_rect.tr.x, glyph_rect.tr.y, 0, 1}},
                cVertex,
                {.raw={tex_rect.tr.x, tex_rect.tr.y, 0, 0}},
                tmvm.v[0],
                tmvm.v[1],
                tmvm.v[2],
                tmvm.v[3],
            {.raw = {glyph_rect.br.x, glyph_rect.br.y, 0, 1}},
                cVertex,
                {.raw={tex_rect.br.x,tex_rect.br.y, 0, 0}},
                tmvm.v[0],
                tmvm.v[1],
                tmvm.v[2],
                tmvm.v[3],

            {.raw = {glyph_rect.br.x, glyph_rect.br.y, 0, 1}},
                cVertex,
                {.raw={tex_rect.br.x,tex_rect.br.y, 0, 0}},
                tmvm.v[0],
                tmvm.v[1],
                tmvm.v[2],
                tmvm.v[3],
            {.raw = {glyph_rect.bl.x, glyph_rect.bl.y, 0, 1}},
                cVertex,
                {.raw={tex_rect.bl.x, tex_rect.bl.y, 0, 0}},
                tmvm.v[0],
                tmvm.v[1],
                tmvm.v[2],
                tmvm.v[3],
            {.raw = {glyph_rect.tl.x, glyph_rect.tl.y, 0, 1}},
                cVertex,
                {.raw={tex_rect.tl.x, tex_rect.tl.y, 0, 0}},
                tmvm.v[0],
                tmvm.v[1],
                tmvm.v[2],
                tmvm.v[3],
        };

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glUniform1i(GfxShader_uniforms[UNIFORM_TEXT_TEXTURE], 0);
        glBindTexture(GL_TEXTURE_2D, texture->gl_tex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        xo += fontchar->advance.x / 64.0;
        c++;
    }

    return;
}

void Graphics_reset_projection_matrix(Graphics *graphics) {
    graphics->projection_matrix = VMatrixIdentity;
}

void Graphics_ortho_projection_matrix(Graphics *graphics, double left,
        double right, double top, double bottom, double near, double far) {
    graphics->projection_matrix =
        VMatrix_make_ortho(left, right, top, bottom, near, far);
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
    check(rc == 1, "Could not build decal shader");
    Hashmap_set(graphics->shaders, bfromcstr("decal"), shader);
    List_push(graphics->shader_list, shader);

    GLuint program = shader->gl_program;
    GfxShader_uniforms[UNIFORM_DECAL_HAS_TEXTURE] =
        glGetUniformLocation(program, "hasTexture");
    GfxShader_uniforms[UNIFORM_DECAL_PROJECTION_MATRIX] =
        glGetUniformLocation(program, "projection");
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
    check(rc == 1, "Could not build tilemap shader");
    Hashmap_set(graphics->shaders, bfromcstr("tilemap"), shader);
    List_push(graphics->shader_list, shader);

    GLuint program = shader->gl_program;
    GfxShader_uniforms[UNIFORM_TILEMAP_MODELVIEW_MATRIX] =
        glGetUniformLocation(program, "modelView");
    GfxShader_uniforms[UNIFORM_TILEMAP_PROJECTION_MATRIX] =
        glGetUniformLocation(program, "projection");
    GfxShader_uniforms[UNIFORM_TILEMAP_TILE_SIZE] =
        glGetUniformLocation(program, "tileSize");
    GfxShader_uniforms[UNIFORM_TILEMAP_SHEET_ROWS_COLS] =
        glGetUniformLocation(program, "sheetRowsCols");
    GfxShader_uniforms[UNIFORM_TILEMAP_SHEET_POT_SIZE] =
        glGetUniformLocation(program, "sheetPotSize");
    GfxShader_uniforms[UNIFORM_TILEMAP_MAP_ROWS_COLS] =
        glGetUniformLocation(program, "mapRowsCols");
    GfxShader_uniforms[UNIFORM_TILEMAP_ATLAS] =
        glGetUniformLocation(program, "atlas");
    GfxShader_uniforms[UNIFORM_TILEMAP_TILESET] =
        glGetUniformLocation(program, "tileset");
    GfxShader_attributes[ATTRIB_TILEMAP_VERTEX] =
        glGetAttribLocation(program, "position");
    GfxShader_attributes[ATTRIB_TILEMAP_TEXTURE] =
        glGetAttribLocation(program, "texture");

    graphics->gen_vao(1, &shader->gl_vertex_array);

    set_up_tilemap_shader(shader, graphics);
    shader->set_up = &set_up_tilemap_shader;
    shader->tear_down = &tear_down_tilemap_shader;
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
    glVertexAttribPointer(GfxShader_attributes[ATTRIB_PARALLAX_VERTEX], 4,
                          GL_FLOAT, GL_FALSE, 0, 0);

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_PARALLAX_TEXTURE], 4,
                          GL_FLOAT, GL_FALSE, 0,
                          (GLvoid *)(sizeof(VVector4) * 4));

    graphics->bind_vao(0);
}

void tear_down_parallax_shader (GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_PARALLAX_VERTEX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_PARALLAX_TEXTURE]);

    graphics->bind_vao(0);
}

void Graphics_build_parallax_shader(Graphics *graphics) {
    GfxShader *shader = calloc(1, sizeof(GfxShader));
    check(shader != NULL, "Could not alloc parallax shader");

    int rc = Graphics_load_shader(graphics, shader_path("parallax.vert"),
        shader_path("parallax.frag"), &shader->gl_program);
    check(rc == 1, "Could not build parallax shader");
    Hashmap_set(graphics->shaders, bfromcstr("parallax"), shader);
    List_push(graphics->shader_list, shader);

    GLuint program = shader->gl_program;
    GfxShader_uniforms[UNIFORM_PARALLAX_MODELVIEW_MATRIX] =
        glGetUniformLocation(program, "modelView");
    GfxShader_uniforms[UNIFORM_PARALLAX_PROJECTION_MATRIX] =
        glGetUniformLocation(program, "projection");
    GfxShader_uniforms[UNIFORM_PARALLAX_TEXTURE] =
        glGetUniformLocation(program, "texture");
    GfxShader_uniforms[UNIFORM_PARALLAX_TEX_PORTION] =
        glGetUniformLocation(program, "texPortion");
    GfxShader_uniforms[UNIFORM_PARALLAX_REPEAT_SIZE] =
        glGetUniformLocation(program, "repeatSize");
    GfxShader_uniforms[UNIFORM_PARALLAX_REPEATS] =
        glGetUniformLocation(program, "repeats");
    GfxShader_uniforms[UNIFORM_PARALLAX_CAMERA_POS] =
        glGetUniformLocation(program, "cameraPos");
    GfxShader_uniforms[UNIFORM_PARALLAX_FACTOR] =
        glGetUniformLocation(program, "parallaxFactor");
    GfxShader_uniforms[UNIFORM_PARALLAX_TEX_SCALE] =
        glGetUniformLocation(program, "texScale");
    GfxShader_attributes[ATTRIB_PARALLAX_VERTEX] =
        glGetAttribLocation(program, "position");
    GfxShader_attributes[ATTRIB_PARALLAX_TEXTURE] =
        glGetAttribLocation(program, "texture");

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
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_COLOR]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_TEX_POS]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX] + 1);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX] + 2);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX] + 3);

    // Position, color, texture, modelView * 4
    int v_size = sizeof(VVector4) * 7;

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TEXT_VERTEX], 4,
                          GL_FLOAT, GL_FALSE, v_size, 0);

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TEXT_COLOR], 4,
                          GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 1));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TEXT_TEX_POS], 4,
                          GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 2));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX],
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 3));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX] + 1,
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 4));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX] + 2,
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 5));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX] + 3,
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 6));

    graphics->bind_vao(0);
}

void tear_down_text_shader(GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_VERTEX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_COLOR]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_TEX_POS]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX] + 1);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX] + 2);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX] + 3);

    graphics->bind_vao(0);
}

void Graphics_build_text_shader(Graphics *graphics) {
    GfxShader *shader = calloc(1, sizeof(GfxShader));
    check(shader != NULL, "Could not alloc text shader");

    int rc = Graphics_load_shader(graphics, shader_path("text.vert"),
        shader_path("text.frag"), &shader->gl_program);
    check(rc == 1, "Could not build text shader");
    Hashmap_set(graphics->shaders, bfromcstr("text"), shader);
    List_push(graphics->shader_list, shader);

    GLuint program = shader->gl_program;
    GfxShader_uniforms[UNIFORM_TEXT_PROJECTION_MATRIX] =
        glGetUniformLocation(program, "projection");
    GfxShader_uniforms[UNIFORM_TEXT_TEXTURE] =
        glGetUniformLocation(program, "texture");
    GfxShader_attributes[ATTRIB_TEXT_VERTEX] =
        glGetAttribLocation(program, "position");
    GfxShader_attributes[ATTRIB_TEXT_COLOR] =
        glGetAttribLocation(program, "color");
    GfxShader_attributes[ATTRIB_TEXT_TEX_POS] =
        glGetAttribLocation(program, "texPos");
    GfxShader_attributes[ATTRIB_TEXT_MODELVIEW_MATRIX] =
        glGetAttribLocation(program, "modelView");

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

    Graphics_build_decal_shader(graphics);
    Graphics_build_tilemap_shader(graphics);
    Graphics_build_parallax_shader(graphics);
    Graphics_build_text_shader(graphics);

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
    if (shader == NULL) {
        graphics->bind_vao(0);

        glUseProgram(0);
    } else {
        graphics->bind_vao(shader->gl_vertex_array);

        glUseProgram(shader->gl_program);
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

GfxTexture *Graphics_texture_from_image(Graphics *graphics, const char *image_name) {
    bstring bimage_name = bfromcstr(image_name);

    void *val = Hashmap_get(graphics->textures, bimage_name);
    if (val != NULL) {
      bdestroy(bimage_name);
      return (GfxTexture *)val;
    }

    GfxTexture *texture = GfxTexture_from_image(image_name);
    Hashmap_set(graphics->textures, bimage_name, texture);

    return texture;
}

Sprite *Graphics_sprite_from_image(Graphics *graphics, const char *image_name,
        GfxSize cell_size, int padding) {
    GfxTexture *texture = Graphics_texture_from_image(graphics, image_name);
    Sprite *sprite = Sprite_create(texture, cell_size, padding);
    check(sprite != NULL, "Couldn't create sprite");

    return sprite;
error:
    return NULL;
}
