#include <lcthw/bstrlib.h>
#include "graphics.h"
#include "sprite.h"

GLint GfxShader_uniforms[NUM_UNIFORMS];
GLint GfxShader_attributes[NUM_ATTRIBUTES];

int Graphics_init_GL(int UNUSED(swidth), int UNUSED(sheight)) {
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
#ifndef DABES_IOS
    glEnable(GL_MULTISAMPLE);
#endif
    glDisable(GL_DEPTH_TEST);
    GLenum error = glGetError();
    check(error == GL_NO_ERROR, "OpenGL init error...");
    return 1;
error:
#ifndef DABES_IOS
    printf("Error initializing OpenGL! %s\n", gluErrorString(error));
#endif
    return 0;
}

GfxSize load_image_dimensions_from_image(char *image_name) {
  GfxSize dimensions = {0,0};
#ifdef DABES_IOS
  unsigned long int *data = NULL;
  GLint size = 0;
  read_file_data(image_name, &data, &size);
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

  unsigned char *old_data = *data;
  unsigned char *resized_data = NULL;
  resized_data = realloc(old_data, pot_w * pot_h * sizeof(unsigned char) * 4);
  check(resized_data != NULL, "Couldn't realloc texture for potize");

  *data = resized_data;
  uint32_t *new_data = (uint32_t *)resized_data;

  *pot_width = pot_w;
  *pot_height = pot_h;

  int last_orig_index = width * height - 1;
  int i = 0;
  for (i = last_orig_index; i >= 0; i--) {
    int old_col = i % width;
    int old_row = i / width;
    int new_index = old_row * pot_w + old_col;
    new_data[new_index] = new_data[i];
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

    glGenTextures(1, &texture->gl_tex);
    glBindTexture(GL_TEXTURE_2D, texture->gl_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    GLenum color_format = GL_RGBA;
#if SDL_BYTEORDER != SDL_BIG_ENDIAN && !defined(DABES_IOS)
    color_format = GL_BGRA;
#endif
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

#ifdef DABES_IOS
GfxTexture *GfxTexture_from_CGImage(CGImageRef image) {
    CGColorSpaceRef colorSpace = NULL;
    check(image != NULL, "No CGImage to load");

    int width = CGImageGetWidth(image);
    int height = CGImageGetHeight(image);
    colorSpace = CGColorSpaceCreateDeviceRGB();
    unsigned char *rawData = calloc(1, height * width * 4);
    check(rawData != NULL, "Couldn't not allocate context buffer");
    int bytesPerPixel = 4;
    int bytesPerRow = bytesPerPixel * width;
    int bitsPerComponent = 8;
    CGContextRef context =
        CGBitmapContextCreate(rawData, width, height, bitsPerComponent,
                              bytesPerRow, colorSpace,
                              kCGImageAlphaPremultipliedLast |
                              kCGBitmapByteOrder32Big);
    CGColorSpaceRelease(colorSpace);
    CGContextDrawImage(context, CGRectMake(0, 0, width, height), image);

    GfxTexture *texture = GfxTexture_from_data(&rawData, width, height,
            GL_RGBA);
    CGContextRelease(context);
    CGImageRelease(image);
    free(rawData);
    return texture;
error:
    if (colorSpace) CGColorSpaceRelease(colorSpace);
    CGImageRelease(image);
    return 0;
}
#endif

#ifdef DABES_SDL
GfxTexture *GfxTexture_from_surface(SDL_Surface *surface) {
    check(surface != NULL, "No surface to load");

    GfxTexture *texture =
        GfxTexture_from_data((unsigned char **)&surface->pixels,
                surface->w, surface->h, GL_RGBA);
    SDL_FreeSurface(surface);
    return texture;
error:
    return 0;
}

SDL_Surface *Graphics_load_SDLImage(char *image_name) {
    SDL_Surface *image = IMG_Load(image_name);
    return image;
}
#endif

#ifdef DABES_IOS
CGImageRef Graphics_load_CGImage(char *image_name) {
    unsigned long int *data = NULL;
    GLint size = 0;
    read_file_data(image_name, &data, &size);
    CFDataRef cf_data = CFDataCreate(NULL, (uint8_t *)data, size);
    free(data);
    CGDataProviderRef provider = CGDataProviderCreateWithCFData(cf_data);
    CGImageRef cg_image =
         CGImageCreateWithPNGDataProvider(provider, NULL, true,
                                          kCGRenderingIntentDefault);
    CGDataProviderRelease(provider);
    CFRelease(cf_data);
    return cg_image;
}
#endif

GfxTexture *GfxTexture_from_image(char *image_name) {
#ifdef DABES_IOS
    CGImageRef cg_image = Graphics_load_CGImage(image_name);
    return GfxTexture_from_CGImage(cg_image);
#else
    SDL_Surface *image = Graphics_load_SDLImage(image_name);
    return GfxTexture_from_surface(image);
#endif
    return NULL;
}

void GfxTexture_destroy(GfxTexture *texture) {
  assert(texture != NULL);

  GLuint gl_textures[] = {texture->gl_tex};
  glDeleteTextures(1, gl_textures);

  free(texture);
}

void Graphics_stroke_rect(Graphics *graphics, VRect rect, GLfloat color[4],
                          double line_width, double rotation) {
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

    GfxUVertex tex = {.raw = {0,0,0,0}};

    glUniform1i(GfxShader_uniforms[UNIFORM_DECAL_HAS_TEXTURE], 0);
    glUniformMatrix4fv(GfxShader_uniforms[UNIFORM_DECAL_PROJECTION_MATRIX], 1,
                       GL_FALSE, graphics->projection_matrix.gl);
    glUniformMatrix4fv(GfxShader_uniforms[UNIFORM_DECAL_MODELVIEW_MATRIX], 1,
                       GL_FALSE, graphics->modelview_matrix.gl);

    GfxUVertex cVertex = {.raw = {color[0], color[1], color[2], color[3]}};

    GfxUVertex vertices[12] = {
      // Vertex
      {.raw = {-w / 2.0, -h / 2.0, 0, 1}},
      {.raw = {w / 2.0, -h / 2.0, 0, 1}},
      {.raw = {w / 2.0, h / 2.0, 0, 1}},
      {.raw = {-w / 2.0, h / 2.0, 0, 1}},

      // Color
      cVertex, cVertex, cVertex, cVertex,

      // Texture
      tex, tex, tex, tex
    };
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GfxUVertex), vertices,
            GL_STATIC_DRAW);

    // Texture
#ifdef DABES_SDL
    glDisable(GL_MULTISAMPLE);
#endif
    glBindTexture(GL_TEXTURE_2D, 0);
    glLineWidth(line_width);
    glDrawArrays(GL_LINE_LOOP, 0, 4);
    return;
#ifdef DABES_SDL
    glEnable(GL_MULTISAMPLE);
#endif
error:
  return;
}

void Graphics_draw_sprite(Graphics *graphics, Sprite *sprite, VRect rect,
        GLfloat color[4], double rot_degs) {
    SpriteFrame *frame = &sprite->frames[sprite->current_frame];
    VPoint frame_offset = frame->offset;
    GfxSize draw_size = sprite->cell_size;

    if (sprite->direction == SPRITE_DIR_FACING_LEFT) {
        frame_offset.x += sprite->cell_size.w;
        draw_size.w = -sprite->cell_size.w;
    }
    Graphics_draw_rect(graphics, rect, color, sprite->texture, frame_offset,
            draw_size, rot_degs);
}

void Graphics_draw_rect(Graphics *graphics, VRect rect, GLfloat color[4],
        GfxTexture *texture, VPoint textureOffset, GfxSize textureSize,
        double rotation) {
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

    GfxUVertex tex_tl = {.raw = {0,0,0,0}};
    GfxUVertex tex_tr = {.raw = {1,0,0,0}};
    GfxUVertex tex_bl = {.raw = {0,1,0,0}};
    GfxUVertex tex_br = {.raw = {1,1,0,0}};
    if (texture) {
        tex_tl.packed.x = textureOffset.x / texture->size.w;
        tex_tl.packed.y = textureOffset.y / texture->size.h;

        tex_tr.packed.x = (textureOffset.x + textureSize.w) / texture->size.w;
        tex_tr.packed.y = textureOffset.y / texture->size.h;

        tex_bl.packed.x = textureOffset.x / texture->size.w;
        tex_bl.packed.y = (textureOffset.y + textureSize.h) / texture->size.h;

        tex_br.packed.x = (textureOffset.x + textureSize.w) / texture->size.w;
        tex_br.packed.y = (textureOffset.y + textureSize.h) / texture->size.h;

        // Remap the texture coords to the power-of-two compatible ones.
        VPoint pot_scale = {
            texture->size.w / texture->pot_size.w,
            texture->size.h / texture->pot_size.h
        };
        tex_tl.packed.x *= pot_scale.x;
        tex_tr.packed.x *= pot_scale.x;
        tex_bl.packed.x *= pot_scale.x;
        tex_br.packed.x *= pot_scale.x;
        tex_tl.packed.y *= pot_scale.y;
        tex_tr.packed.y *= pot_scale.y;
        tex_bl.packed.y *= pot_scale.y;
        tex_br.packed.y *= pot_scale.y;
    }

    glUniform1i(GfxShader_uniforms[UNIFORM_DECAL_HAS_TEXTURE],
                texture ? texture->gl_tex : 0);
    glUniformMatrix4fv(GfxShader_uniforms[UNIFORM_DECAL_PROJECTION_MATRIX], 1,
                       GL_FALSE, graphics->projection_matrix.gl);
    glUniformMatrix4fv(GfxShader_uniforms[UNIFORM_DECAL_MODELVIEW_MATRIX], 1,
                       GL_FALSE, graphics->modelview_matrix.gl);

    GfxUVertex cVertex = {.raw = {color[0], color[1], color[2], color[3]}};

    GfxUVertex vertices[12] = {
      // Vertex
      {.raw = {-w / 2.0, -h / 2.0, 0, 1}},
      {.raw = {w / 2.0, -h / 2.0, 0, 1}},
      {.raw = {-w / 2.0, h / 2.0, 0, 1}},
      {.raw = {w / 2.0, h / 2.0, 0, 1}},

      // Color
      cVertex, cVertex, cVertex, cVertex,

      // Texture
      tex_tl, tex_tr, tex_bl, tex_br
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture ? texture->gl_tex : 0);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindTexture(GL_TEXTURE_2D, 0);
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

void set_up_decal_shader(GfxShader *UNUSED(shader)) {
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_VERTEX]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_COLOR]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_TEXTURE]);
    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_VERTEX], 4,
                          GL_FLOAT, GL_FALSE, 0, 0);

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_COLOR], 4,
                          GL_FLOAT, GL_FALSE, 0,
                          (GLvoid *)(sizeof(GfxUVertex) * 4));

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_DECAL_TEXTURE], 4,
                          GL_FLOAT, GL_FALSE,
                          0, (GLvoid *)(sizeof(GfxUVertex) * 8));
}

void tear_down_decal_shader(GfxShader *UNUSED(shader)) {
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_VERTEX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_COLOR]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_DECAL_TEXTURE]);
}

void Graphics_build_decal_shader(Graphics *graphics) {
    GfxShader *shader = malloc(sizeof(GfxShader));
    check(shader != NULL, "Could not alloc decal shader");

    int rc = Graphics_load_shader(graphics, shader_path("decal.vert"),
        shader_path("decal.frag"), &shader->gl_program);
    check(rc == 1, "Could not build decal shader");
    Hashmap_set(graphics->shaders, bfromcstr("decal"), shader);

    GLuint program = shader->gl_program;
    GfxShader_uniforms[UNIFORM_DECAL_HAS_TEXTURE] =
        glGetUniformLocation(program, "hasTexture");
    GfxShader_uniforms[UNIFORM_DECAL_MODELVIEW_MATRIX] =
        glGetUniformLocation(program, "modelView");
    GfxShader_uniforms[UNIFORM_DECAL_PROJECTION_MATRIX] =
        glGetUniformLocation(program, "projection");
    GfxShader_attributes[ATTRIB_DECAL_VERTEX] =
        glGetAttribLocation(program, "position");
    GfxShader_attributes[ATTRIB_DECAL_COLOR] =
        glGetAttribLocation(program, "color");

    shader->set_up = &set_up_decal_shader;
    shader->tear_down = &tear_down_decal_shader;
    return;
error:
    if (shader) free(shader);
    return;
}

void set_up_tilemap_shader(GfxShader *UNUSED(shader)) {
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TILEMAP_VERTEX]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_TILEMAP_TEXTURE]);
    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TILEMAP_VERTEX], 4,
                          GL_FLOAT, GL_FALSE, 0, 0);

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_TILEMAP_TEXTURE], 4,
                          GL_FLOAT, GL_FALSE, 0,
                          (GLvoid *)(sizeof(GfxUVertex) * 4));
}

void tear_down_tilemap_shader (GfxShader *UNUSED(shader)) {
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TILEMAP_VERTEX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_TILEMAP_TEXTURE]);
}

void Graphics_build_tilemap_shader(Graphics *graphics) {
    GfxShader *shader = calloc(1, sizeof(GfxShader));
    check(shader != NULL, "Could not alloc tilemap shader");

    int rc = Graphics_load_shader(graphics, shader_path("tilemap.vert"),
        shader_path("tilemap.frag"), &shader->gl_program);
    check(rc == 1, "Could not build tilemap shader");
    Hashmap_set(graphics->shaders, bfromcstr("tilemap"), shader);

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

    shader->set_up = &set_up_tilemap_shader;
    shader->tear_down = &tear_down_tilemap_shader;
    return;
error:
    if (shader) free(shader);
    return;
}

void set_up_parallax_shader(GfxShader *UNUSED(shader)) {
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_PARALLAX_VERTEX]);
    glEnableVertexAttribArray(GfxShader_attributes[ATTRIB_PARALLAX_TEXTURE]);
    glVertexAttribPointer(GfxShader_attributes[ATTRIB_PARALLAX_VERTEX], 4,
                          GL_FLOAT, GL_FALSE, 0, 0);

    glVertexAttribPointer(GfxShader_attributes[ATTRIB_PARALLAX_TEXTURE], 4,
                          GL_FLOAT, GL_FALSE, 0,
                          (GLvoid *)(sizeof(GfxUVertex) * 4));
}

void tear_down_parallax_shader (GfxShader *UNUSED(shader)) {
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_PARALLAX_VERTEX]);
    glDisableVertexAttribArray(GfxShader_attributes[ATTRIB_PARALLAX_TEXTURE]);
}

void Graphics_build_parallax_shader(Graphics *graphics) {
    GfxShader *shader = calloc(1, sizeof(GfxShader));
    check(shader != NULL, "Could not alloc parallax shader");

    int rc = Graphics_load_shader(graphics, shader_path("parallax.vert"),
        shader_path("parallax.frag"), &shader->gl_program);
    check(rc == 1, "Could not build parallax shader");
    Hashmap_set(graphics->shaders, bfromcstr("parallax"), shader);

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

    shader->set_up = &set_up_parallax_shader;
    shader->tear_down = &tear_down_parallax_shader;
    return;
error:
    if (shader) free(shader);
    return;
}

int Graphics_init(void *self) {
    Graphics *graphics = self;
#ifndef DABES_IOS
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

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    graphics->current_shader = NULL;
    graphics->shaders = Hashmap_create(NULL, NULL);

    Graphics_build_decal_shader(graphics);
    Graphics_build_tilemap_shader(graphics);
    Graphics_build_parallax_shader(graphics);

    graphics->textures = Hashmap_create(NULL, NULL);
    graphics->sprites = Hashmap_create(NULL, NULL);

  return 1;
}

void Graphics_destroy(void *self) {
    Graphics *graphics = self;
#ifndef DABES_IOS
    TTF_CloseFont(graphics->debug_text_font);
#endif

    Hashmap_destroy(graphics->textures,
                    (Hashmap_destroy_func)GfxTexture_destroy);
    GLuint textures[] = {graphics->debug_text_texture};
    glDeleteTextures(1, textures);
    free(graphics);
}

GLuint Graphics_load_shader(Graphics *UNUSED(graphics), char *vert_name,
        char *frag_name, GLuint *compiled_program) {
    GLuint vertex_shader, fragment_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    check(vertex_shader != 0, "Couldn't create vertex shader");
    check(fragment_shader != 0, "Couldn't create fragment shader");

    GLchar *v_src, *f_src;
    GLint v_size, f_size;
    read_text_file(vert_name, &v_src, &v_size);
    read_text_file(frag_name, &f_src, &f_size);

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

    //if(strcmp(vert_name, shader_path("decal.vert")) == 0) return 1;
    *compiled_program = program;
    return 1;
error:
    return 0;
}

GfxShader *Graphics_get_shader(Graphics *graphics, char *name) {
  bstring key = bfromcstr(name);
  GfxShader *val = Hashmap_get(graphics->shaders, key);
  if (val) return val;
  return NULL;
}

void Graphics_use_shader(Graphics *graphics, GfxShader *shader) {
  if (graphics->current_shader) {
    graphics->current_shader->tear_down(graphics->current_shader);
  }

  graphics->current_shader = NULL;

  if (shader != NULL) {
    glUseProgram(shader->gl_program);
    shader->set_up(shader);
    graphics->current_shader = shader;
  } else {
    glUseProgram(0);
  }
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

GfxTexture *Graphics_texture_from_image(Graphics *graphics, char *image_name) {
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

Sprite *Graphics_sprite_from_image(Graphics *graphics, char *image_name,
        GfxSize cell_size) {
    GfxTexture *texture = Graphics_texture_from_image(graphics, image_name);
    Sprite *sprite = Sprite_create(texture, cell_size);
    check(sprite != NULL, "Couldn't create sprite");

    return sprite;
error:
    return NULL;
}

Object GraphicsProto = {
    .init = Graphics_init
};
