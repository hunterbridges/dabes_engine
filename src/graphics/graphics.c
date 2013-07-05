#ifdef DABES_IOS
#include <OpenGLES/ES2/glext.h>
#endif
#ifdef DABES_MAC
#include <OpenGL/glext.h>
#endif
#include <lcthw/bstrlib.h>
#include "draw_buffer.h"
#include "graphics.h"
#include "stb_image.h"
#include "sprite.h"
#include "../core/engine.h"

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

/*
GfxSize load_image_dimensions_from_image(char *image_name) {
  GfxSize dimensions = {0,0};
#if defined(DABES_IOS) || defined(DABES_MAC)
  unsigned char *data = NULL;
  GLint size = 0;
  Engine_read_file(image_name, &data, &size);
  CFDataRef cf_data = CFDataCreate(NULL, (uint8_t *)data, size);
  free(data);
  CGDataProviderRef provider = CGDataProviderCreateWithCFData(cf_data);
  CGImageRef cg_image =
      CGImageCreateWithPNGDataProvider(provider, NULL, true,
                                       kCGRenderingIntentDefault);
  CGDataProviderRelease(provider);
  CFRelease(cf_data);
  dimensions.w = CGImageGetWidth(cg_image);
  dimensions.h = CGImageGetHeight(cg_image);
  CGImageRelease(cg_image);
#else
  SDL_Surface *image = IMG_Load(image_name);
  dimensions.w = image->w;
  dimensions.h = image->h;
  SDL_FreeSurface(image);
#endif
  return dimensions;
}
 */

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

static inline int data_potize(unsigned char **data, int width, int height,
                              int *pot_width, int *pot_height) {
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

  int last_orig_index = (width * height - 1) * 4;
  int i = 0;
  for (i = last_orig_index; i >= 0; i -= 4) {
    int idx = i / 4;
    int old_col = idx % width;
    int old_row = idx / width;
    int new_index = old_row * pot_d + old_col;

    new_data[new_index * 4] = new_data[i];         // R
    new_data[new_index * 4 + 1] = new_data[i + 1]; // G
    new_data[new_index * 4 + 2] = new_data[i + 2]; // B
    new_data[new_index * 4 + 3] = new_data[i + 3]; // A
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
    int rc = data_potize(data, width, height, &pot_width, &pot_height);
    check(rc == 1, "Could not potize data");

    texture->pot_size.w = pot_width;
    texture->pot_size.h = pot_height;

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture->gl_tex);
    glBindTexture(GL_TEXTURE_2D, texture->gl_tex);
    GLenum color_format = GL_RGBA;
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, source_format, pot_width,
                 pot_height, 0, color_format,
                 GL_UNSIGNED_BYTE, *data);
    GLenum er = glGetError();
    if (er != GL_NO_ERROR) {
        printf("%d\n", GL_MAX_TEXTURE_SIZE);
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

    // Transpose modelview matrix because attribute reads columns, not rows.
    VMatrix tmvm = graphics->modelview_matrix;

    VVector4 vertices[7 * num_points];
    int i = 0;
    for (i = 0; i < num_points; i++) {
        int pos_idx   = i * 7;
        int color_idx = i * 7 + 1;
        int tex_idx   = i * 7 + 2;
        int mvm_1_idx = i * 7 + 3;
        int mvm_2_idx = i * 7 + 4;
        int mvm_3_idx = i * 7 + 5;
        int mvm_4_idx = i * 7 + 6;

        VPoint point = points[i];
        VVector4 pos = {.raw = {point.x, point.y, 0.0, 1.0}};
        vertices[pos_idx] = pos;
        vertices[color_idx] = cVertex;
        vertices[tex_idx] = tex;
        vertices[mvm_1_idx] = tmvm.v[0];
        vertices[mvm_2_idx] = tmvm.v[1];
        vertices[mvm_3_idx] = tmvm.v[2];
        vertices[mvm_4_idx] = tmvm.v[3];
    }
    glBufferData(GL_ARRAY_BUFFER, 7 * num_points * sizeof(VVector4), vertices,
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
                          GLfloat color[4], double rot_degs, int z_index) {
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
                       frame_offset, draw_size, rot_degs, z_index);
}

void Graphics_draw_rect(Graphics *graphics, struct DrawBuffer *draw_buffer,
        VRect rect, GLfloat color[4], GfxTexture *texture, VPoint textureOffset,
        GfxSize textureSize, double rotation, int z_index) {
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

    // Transpose modelview matrix because attribute reads columns, not rows.
    VMatrix tmvm = graphics->modelview_matrix;

    VVector4 vertices[7 * 6] = {
        {.raw = {-w / 2.0, -h / 2.0, 0, 1}},
            cVertex,
            {.raw={tex_rect.tl.x, tex_rect.tl.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],
        {.raw = {w / 2.0, -h / 2.0, 0, 1}},
            cVertex,
            {.raw={tex_rect.tr.x, tex_rect.tr.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],
        {.raw = {w / 2.0, h / 2.0, 0, 1}},
            cVertex,
            {.raw={tex_rect.br.x,tex_rect.br.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],

        {.raw = {w / 2.0, h / 2.0, 0, 1}},
            cVertex,
            {.raw={tex_rect.br.x,tex_rect.br.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],
        {.raw = {-w / 2.0, h / 2.0, 0, 1}},
            cVertex,
            {.raw={tex_rect.bl.x, tex_rect.bl.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],
        {.raw = {-w / 2.0, -h / 2.0, 0, 1}},
            cVertex,
            {.raw={tex_rect.tl.x, tex_rect.tl.y, 0, 0}},
            tmvm.v[0],
            tmvm.v[1],
            tmvm.v[2],
            tmvm.v[3],
    };

    if (draw_buffer) {
        DrawBuffer_buffer(draw_buffer, texture, z_index, 6, 7, vertices);
    } else {
        glUniform1i(GfxShader_uniforms[UNIFORM_DECAL_HAS_TEXTURE],
                    texture ? texture->gl_tex : 0);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glActiveTexture(GL_TEXTURE0);
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

void Graphics_draw_debug_text(Graphics *UNUSED(graphics),
        int UNUSED(ticks_since_last)) {
    return;
#if 0
    Graphics_reset_projection_matrix(graphics);
    GLuint textures[] = {graphics->debug_text_texture};
    if (graphics->debug_text_texture != 0) glDeleteTextures(1, textures);
    graphics->debug_text_texture = 0;

    SDL_Rect debugRect = {0, 0, 160, 10};

    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, debugRect.w,
            debugRect.h, 32, rmask, gmask, bmask, amask);

    Uint32 uBlack = SDL_MapRGBA(surface->format, 0, 0, 0, 255);
    SDL_FillRect(surface, &debugRect, uBlack);

    SDL_Color txtBlack = {255,255,255,255};
    char *dTxt = malloc(256 * sizeof(char));
    sprintf(dTxt, "FPS CAP: %d           ACTUAL: %d", FPS,
            (int)ceil(1000.0 / ticks_since_last));
    SDL_Surface *debugText = TTF_RenderText_Solid(graphics->debug_text_font,
            dTxt, txtBlack);
    free(dTxt);

    SDL_BlitSurface(debugText, NULL, surface, &debugRect);
    SDL_FreeSurface(debugText);
    graphics->debug_text_texture = load_surface_as_texture(surface);

    VRect rect = VRect_from_SDL_Rect(debugRect);
    GLfloat glBlack[4] = {0,0,0,255};
    Graphics_draw_rect(graphics, rect, glBlack, graphics->debug_text_texture, 0);
#endif
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

void set_up_decal_shader(GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_VERTEX]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_COLOR]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_TEXTURE]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 1);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 2);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 3);

    // Position, color, texture, modelView * 4
    int v_size = sizeof(VVector4) * 7;

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_VERTEX], 4,
                          GL_FLOAT, GL_FALSE, v_size, 0);

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_COLOR], 4,
                          GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 1));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_TEXTURE], 4,
                          GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 2));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX],
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 3));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 1,
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 4));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 2,
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 5));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_MODELVIEW_MATRIX] + 3,
                          4, GL_FLOAT, GL_FALSE, v_size,
                          (GLvoid *)(sizeof(VVector4) * 6));

    graphics->bind_vao(0);
}

void tear_down_decal_shader(GfxShader *shader, Graphics *graphics) {
    graphics->bind_vao(shader->gl_vertex_array);

    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_VERTEX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_COLOR]);
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

Graphics *Graphics_create(Engine *engine) {
    Graphics *graphics = calloc(1, sizeof(Graphics));
    check(graphics != NULL, "Couldn't create graphics");
  
    graphics->engine = engine;
  
#ifdef DABES_SDL
    graphics->debug_text_font = TTF_OpenFont("media/fonts/uni.ttf", 8);
#endif
    graphics->debug_text_texture = 0;
    graphics->screen_size.w = SCREEN_WIDTH;
    graphics->screen_size.h = SCREEN_HEIGHT;
    glGenBuffers(1, &graphics->array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->array_buffer);
    glClearColor(0.65f, 0.65f, 0.65f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    graphics->current_shader = NULL;
    graphics->shaders = Hashmap_create(NULL, NULL);
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

    graphics->textures = Hashmap_create(NULL, NULL);
    graphics->sprites = Hashmap_create(NULL, NULL);

    return graphics;
error:
    return NULL;
}

void nulldestroy(void *obj) { }

void Graphics_destroy(Graphics *graphics) {
#ifdef DABES_SDL
    TTF_CloseFont(graphics->debug_text_font);
#endif
  
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
  
    GLuint textures[] = {graphics->debug_text_texture};
    glDeleteTextures(1, textures);
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
