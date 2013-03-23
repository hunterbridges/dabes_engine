#include "graphics.h"

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    static Uint32 rmask = 0xff000000;
    static Uint32 gmask = 0x00ff0000;
    static Uint32 bmask = 0x0000ff00;
    static Uint32 amask = 0x000000ff;
#else
    static Uint32 rmask = 0x000000ff;
    static Uint32 gmask = 0x0000ff00;
    static Uint32 bmask = 0x00ff0000;
    static Uint32 amask = 0xff000000;
#endif

// uniform index
enum {
	UNIFORM_PROJECTION_MATRIX,
	UNIFORM_MODELVIEW_MATRIX,
	NUM_UNIFORMS
} UNIFORMS;
GLint uniforms[NUM_UNIFORMS];

// attribute index
enum {
	ATTRIB_VERTEX,
	ATTRIB_COLOR,
	NUM_ATTRIBUTES
} ATTRIBS;
GLint attributes[NUM_ATTRIBUTES];

void mat4f_MultiplyMat4f(const float* a, const float* b, float* mout)
{
  mout[0]  = a[0] * b[0]  + a[4] * b[1]  + a[8] * b[2]   + a[12] * b[3];
  mout[1]  = a[1] * b[0]  + a[5] * b[1]  + a[9] * b[2]   + a[13] * b[3];
  mout[2]  = a[2] * b[0]  + a[6] * b[1]  + a[10] * b[2]  + a[14] * b[3];
  mout[3]  = a[3] * b[0]  + a[7] * b[1]  + a[11] * b[2]  + a[15] * b[3];

  mout[4]  = a[0] * b[4]  + a[4] * b[5]  + a[8] * b[6]   + a[12] * b[7];
  mout[5]  = a[1] * b[4]  + a[5] * b[5]  + a[9] * b[6]   + a[13] * b[7];
  mout[6]  = a[2] * b[4]  + a[6] * b[5]  + a[10] * b[6]  + a[14] * b[7];
  mout[7]  = a[3] * b[4]  + a[7] * b[5]  + a[11] * b[6]  + a[15] * b[7];

  mout[8]  = a[0] * b[8]  + a[4] * b[9]  + a[8] * b[10]  + a[12] * b[11];
  mout[9]  = a[1] * b[8]  + a[5] * b[9]  + a[9] * b[10]  + a[13] * b[11];
  mout[10] = a[2] * b[8]  + a[6] * b[9]  + a[10] * b[10] + a[14] * b[11];
  mout[11] = a[3] * b[8]  + a[7] * b[9]  + a[11] * b[10] + a[15] * b[11];

  mout[12] = a[0] * b[12] + a[4] * b[13] + a[8] * b[14]  + a[12] * b[15];
  mout[13] = a[1] * b[12] + a[5] * b[13] + a[9] * b[14]  + a[13] * b[15];
  mout[14] = a[2] * b[12] + a[6] * b[13] + a[10] * b[14] + a[14] * b[15];
  mout[15] = a[3] * b[12] + a[7] * b[13] + a[11] * b[14] + a[15] * b[15];
}

GfxRect GfxRect_from_xywh(double x, double y, double w, double h) {
    GfxPoint tl = { x, y };
    GfxPoint tr = { x + w, y };
    GfxPoint bl = { x, y + h };
    GfxPoint br = { x + w, y + h };
    GfxRect rect = {
        .tl = tl,
        .tr = tr,
        .bl = bl,
        .br = br
    };
    return rect;
}

GfxRect GfxRect_from_SDL_Rect(SDL_Rect rect) {
    return GfxRect_from_xywh(rect.x, rect.y, rect.w, rect.h);
}

void Graphics_draw_rect(Graphics *graphics, GfxRect rect, GLfloat color[4],
        GLuint texture, double rotation) {
    check_mem(graphics);
#pragma message("REIMPLIMENT")
    Graphics_reset_modelview_matrix(graphics);
    double w = rect.tr.x - rect.tl.x;
    double h = rect.bl.y - rect.tl.y;

    GfxPoint center = {
        rect.tl.x + w / 2,
        rect.tl.y + h / 2
    };
    Graphics_translate_modelview_matrix(graphics, -SCREEN_WIDTH/2,
                                        -SCREEN_HEIGHT/2, 0.f);
    Graphics_translate_modelview_matrix(graphics, center.x, center.y, 0.f);
    Graphics_rotate_modelview_matrix(graphics, rotation, 0, 0, 1);

#ifdef DABES_IOS
    glUniformMatrix4fv(uniforms[UNIFORM_PROJECTION_MATRIX], 1,
                       GL_FALSE, graphics->projection_matrix.gl);
    glUniformMatrix4fv(uniforms[UNIFORM_MODELVIEW_MATRIX], 1,
                       GL_FALSE, graphics->modelview_matrix.gl);
    GfxUVertex cVertex = {color[0], color[1], color[2], color[3]};

    GfxUVertex vertices[8] = {
      {-w / 2.0, -h / 2.0, 0, 1},
      {w / 2.0, -h / 2.0, 0, 1},
      {-w / 2.0, h / 2.0, 0, 1},
      {w / 2.0, h / 2.0, 0, 1},
      cVertex, cVertex, cVertex, cVertex
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Texture
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#else
    glColor4fv(color);
    glBindTexture(GL_TEXTURE_2D, texture);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBegin(GL_TRIANGLE_STRIP);
        glTexCoord2f(0, 0);
        glVertex2f(-w / 2.0, -h / 2.0);
        glTexCoord2f(1, 0);
        glVertex2f(w / 2.0, -h / 2.0);
        glTexCoord2f(0, 1);
        glVertex2f(-w / 2.0, h / 2.0);
        glTexCoord2f(1, 1);
        glVertex2f(w / 2.0, h / 2.0);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
#endif
  return;
error:
    return;
}

void Graphics_draw_debug_text(Graphics *graphics,
        int ticks_since_last) {
#ifndef DABES_IOS
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

    GfxRect rect = GfxRect_from_SDL_Rect(debugRect);
    GLfloat glBlack[4] = {0,0,0,255};
    Graphics_scale_projection_matrix(graphics, 1);
    Graphics_draw_rect(graphics, rect, glBlack, graphics->debug_text_texture, 0);
#endif
}


GfxTransform3D GfxTransform3D_ortho(float left, float right, float bottom,
                                    float top, float near, float far) {
  GfxTransform3D ortho = {
    2 / (right - left), 0, 0, -1 * (right + left) / (right - left),
    0, 2 / (top - bottom), 0, -1 * (top + bottom) / (top - bottom),
    0, 0, -2 / (far - near), far * near / (far - near),
    0, 0, 0, 1
  };
  return ortho;
}

void Graphics_scale_projection_matrix(Graphics *graphics, double scale) {
    double left = graphics->screen_size.w / (-2 * scale);
    double right = graphics->screen_size.w / (2 * scale);
    double bottom = graphics->screen_size.h / (2 * scale);
    double top = graphics->screen_size.h / (-2 * scale);

    GfxUMatrix ortho;
    ortho.gfx = GfxTransform3D_ortho(left, right, bottom, top, 1.0, -1.0);
    graphics->projection_matrix = ortho;
#ifndef DABES_IOS
    glOrtho(left, right, bottom, top, 1.0, -1.0 );
#endif
}

void Graphics_reset_projection_matrix(Graphics *graphics) {
#ifdef DABES_IOS
    GfxUMatrix identity;
    identity.ca = CATransform3DIdentity;
    graphics->projection_matrix = identity;
#else
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
#endif
}

void Graphics_reset_modelview_matrix(Graphics *graphics) {
#ifdef DABES_IOS
    GfxUMatrix identity;
    identity.ca = CATransform3DIdentity;
    graphics->modelview_matrix = identity;
#else
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
#endif
}

static inline GfxUMatrix GFXUMatrix_rotate(GfxUMatrix matrix,
                                           double rot_degs,
                                           double x, double y, double z) {
#ifdef DABES_IOS
    GfxUMatrix rotated;
    rotated.ca =
        CATransform3DRotate(matrix.ca, 180.0 * rot_degs / M_PI, x, y, z);
    return rotated;
#else
    glRotatef(rot_degs, x, y, z);
    return matrix;
#endif
}

void Graphics_rotate_projection_matrix(Graphics *graphics, double rot_degs,
                                       double x, double y, double z) {

#ifndef DABES_IOS
    glMatrixMode(GL_PROJECTION);
#endif
    graphics->projection_matrix =
        GFXUMatrix_rotate(graphics->projection_matrix, rot_degs, x, y, z);
}

void Graphics_rotate_modelview_matrix(Graphics *graphics, double rot_degs,
                                      double x, double y, double z) {
#ifndef DABES_IOS
    glMatrixMode(GL_MODELVIEW);
#endif
    graphics->modelview_matrix =
        GFXUMatrix_rotate(graphics->modelview_matrix, rot_degs, x, y, z);
}

static inline GfxUMatrix GFXUMatrix_translate(GfxUMatrix matrix, double x,
                                              double y, double z) {
#ifdef DABES_IOS
    GfxUMatrix xformed;
    xformed.ca = CATransform3DTranslate(matrix.ca, x, y, z);
    return xformed;
#else
    glTranslatef(x, y, z);
    return matrix;
#endif
}

void Graphics_translate_modelview_matrix(Graphics *graphics,
                                         double x, double y, double z) {
#ifndef DABES_IOS
    glMatrixMode(GL_MODELVIEW);
#endif
    graphics->modelview_matrix =
        GFXUMatrix_translate(graphics->modelview_matrix, x, y, z);
}


int Graphics_init(void *self) {
    Graphics *graphics = self;
#ifndef DABES_IOS
    graphics->debug_text_font = TTF_OpenFont("media/fonts/uni.ttf", 8);
#endif
    graphics->debug_text_texture = 0;
    graphics->screen_size.w = SCREEN_WIDTH;
    graphics->screen_size.h = SCREEN_HEIGHT;
    graphics->shader = 0;
    glGenBuffers(1, &graphics->array_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, graphics->array_buffer);
    glClearColor(0.65f, 0.65f, 0.65f, 1.0f);

    Graphics_load_shader(graphics, shader_path("decal.vert"),
        shader_path("decal.frag"));

  return 1;
}

void Graphics_destroy(void *self) {
    Graphics *graphics = self;
#ifndef DABES_IOS
    TTF_CloseFont(graphics->debug_text_font);
#endif
    GLuint textures[] = {graphics->debug_text_texture};
    glDeleteTextures(1, textures);
    free(graphics);
}

GLuint Graphics_load_shader(Graphics *graphics, char *vert_name,
        char *frag_name) {
    GLuint vertex_shader, fragment_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    check(vertex_shader != 0, "Couldn't create vertex shader");
    check(fragment_shader != 0, "Couldn't create fragment shader");

    GLuint *last_shader = NULL;
    GLchar *v_src, *f_src;
    GLint v_size, f_size;
    read_text_file(vert_name, &v_src, &v_size);
    read_text_file(frag_name, &f_src, &f_size);

    glShaderSource(vertex_shader, 1, (const GLchar**)&v_src, &v_size);
    glShaderSource(fragment_shader, 1, (const GLchar**)&f_src, &f_size);
    free(v_src);
    free(f_src);

    GLint compiled = 0;
    last_shader = &vertex_shader;

    glCompileShader(vertex_shader);
    glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compiled);
    Graphics_log_shader(vertex_shader);
    check(compiled == GL_TRUE, "Vertex shader failed to compile");

    last_shader = &fragment_shader;
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
    Graphics_log_shader(fragment_shader);
    check(compiled == GL_TRUE, "Fragment shader failed to compile");
    last_shader = NULL;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
#ifdef DABES_IOS
    glBindAttribLocation(program, ATTRIB_VERTEX, "position");
    glBindAttribLocation(program, ATTRIB_COLOR, "color");
#endif
    glLinkProgram(program);
    Graphics_log_program(program);

    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    check(linked == 1, "Linking shader program");

#ifdef DABES_IOS
    uniforms[UNIFORM_MODELVIEW_MATRIX] =
        glGetUniformLocation(program, "modelView");
    uniforms[UNIFORM_PROJECTION_MATRIX] =
        glGetUniformLocation(program, "projection");
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    glEnableVertexAttribArray(ATTRIB_COLOR);
    glVertexAttribPointer(ATTRIB_VERTEX, 4, GL_FLOAT, GL_FALSE, 0,
                          0);

    glVertexAttribPointer(ATTRIB_COLOR, 4, GL_FLOAT, GL_FALSE,
                          0, (GLvoid *)(sizeof(GfxUVertex) * 4));
#endif

  // TODO: Hashmap
    graphics->shader = program;
    glUseProgram(program);
    return 1;
error:
    return 0;
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

Object GraphicsProto = {
    .init = Graphics_init
};
