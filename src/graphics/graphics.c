#include "graphics.h"

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

void Graphics_draw_rect(Graphics *graphics, GfxRect rect, GLdouble color[4],
        GLuint texture, double rotation) {
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    double w = rect.tr.x - rect.tl.x;
    double h = rect.bl.y - rect.tl.y;

    GfxPoint center = {
        rect.tl.x + w / 2,
        rect.tl.y + h / 2
    };
    glTranslatef(-SCREEN_WIDTH/2, -SCREEN_HEIGHT/2, 0.f);
    glTranslatef(center.x,center.y, 0.f);
    glRotatef(rotation, 0, 0, 1);

    glColor4dv(color);
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
}

void Graphics_draw_debug_text(Graphics *graphics,
        long unsigned int ticks_since_last) {
    GLuint textures[] = {graphics->debug_text_texture};
    if (graphics->debug_text_texture != 0) glDeleteTextures(1, textures);
    graphics->debug_text_texture = 0;

    SDL_Rect debugRect = {0, 0, 160, 10};

    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif
    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_HWSURFACE, debugRect.w,
            debugRect.h, 32, rmask, gmask, bmask, amask);

    Uint32 uBlack = SDL_MapRGBA(surface->format, 0, 0, 0, 255);
    SDL_FillRect(surface, &debugRect, uBlack);

    SDL_Color txtBlack = {255,255,255,255};
    char *dTxt = malloc(256 * sizeof(char));
    sprintf(dTxt, "FPS CAP: %d           ACTUAL: %d", FPS,
            (int)ceil(1000.0 / ticks_since_last));
    //debug("%s", dTxt);
    SDL_Surface *debugText = TTF_RenderText_Solid(graphics->debug_text_font,
            dTxt, txtBlack);
    free(dTxt);

    SDL_BlitSurface(debugText, NULL, surface, &debugRect);
    SDL_FreeSurface(debugText);
    graphics->debug_text_texture = load_surface_as_texture(surface);
    //debug("%d", graphics->debug_text_texture);

    GfxRect rect = GfxRect_from_SDL_Rect(debugRect);
    GLdouble glBlack[4] = {0,0,0,255};
    Graphics_scale_projection_matrix(graphics, 1);
    Graphics_draw_rect(graphics, rect, glBlack, graphics->debug_text_texture, 0);
}

void Graphics_scale_projection_matrix(Graphics *graphics, double scale) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(graphics->screen_size.w / (-2 * scale),
            graphics->screen_size.w / (2 * scale),
            graphics->screen_size.h / (2 * scale),
            graphics->screen_size.h / (-2 * scale),
            1.0, -1.0 );
}

int Graphics_init(void *self) {
    Graphics *graphics = self;
    graphics->debug_text_font = TTF_OpenFont("media/fonts/uni.ttf", 8);
    graphics->debug_text_texture = 0;
    graphics->screen_size.w = SCREEN_WIDTH;
    graphics->screen_size.h = SCREEN_HEIGHT;
    graphics->shader = 0;
    Graphics_load_shader(graphics, "media/shaders/decal.vert",
        "media/shaders/decal.frag");
    return 1;
}

void Graphics_destroy(void *self) {
    Graphics *graphics = self;
    TTF_CloseFont(graphics->debug_text_font);
    GLuint textures[] = {graphics->debug_text_texture};
    glDeleteTextures(1, textures);
    free(graphics);
}

GLuint Graphics_load_shader(Graphics *graphics, char *vert_name,
        char *frag_name) {
    GLuint vertex_shader, fragment_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);

    GLuint *last_shader = NULL;
    GLchar *v_src, *f_src;
    GLuint v_size, f_size;
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
    check(compiled == 1, "Vertex shader compile");

    last_shader = &fragment_shader;
    glCompileShader(fragment_shader);
    glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
    Graphics_log_shader(fragment_shader);
    check(compiled == 1, "Fragment shader compile");
    last_shader = NULL;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
    Graphics_log_program(program);

    GLuint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    check(linked == 1, "Linking shader program");

    graphics->shader = program;
    printf("SHADER LINKD!\n");
    return 1;
error: {
    return 0;
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

Object GraphicsProto = {
    .init = Graphics_init
};
