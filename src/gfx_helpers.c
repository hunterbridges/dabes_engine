#include "gfx_helpers.h"

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

int init_GL(int swidth, int sheight) {
    glEnable(GL_TEXTURE_2D);
#ifndef DABES_IOS
    glEnable(GL_MULTISAMPLE);
#endif
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    GLenum error = glGetError();
    check(error == GL_NO_ERROR, "OpenGL init error...");
    return 1;
error:
#ifndef DABES_IOS
    printf("Error initializing OpenGL! %s\n", gluErrorString(error));
#endif
    return 0;
}

GLuint load_surface_as_texture(SDL_Surface *surface) {
    check(surface != NULL, "No surface to load");

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GLenum color_format = GL_RGBA;
#if SDL_BYTEORDER != SDL_BIG_ENDIAN && !defined(DABES_IOS)
    color_format = GL_BGRA;
#endif
#ifndef DABES_IOS
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
#endif
    glTexImage2D(GL_TEXTURE_2D, 0, 4, surface->w, surface->h, 0,
        color_format, GL_UNSIGNED_BYTE, surface->pixels);
    SDL_FreeSurface(surface);
    return texture;
error:
    return 0;
}

// TODO: Hashmap
GLuint load_image_as_texture(char *image_name) {
#ifndef DABES_IOS
    SDL_Surface *image = IMG_Load(image_name);
    return load_surface_as_texture(image);
#endif
    return 0;
}

SDL_Surface *gradient(unsigned int width, unsigned int height) {
    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width,
            height, 32, rmask, gmask, bmask, amask);
    unsigned int i = 0;
    for (i = 0; i < height; i++) {
        int gradation = 255 * (height - i) / height;
        Uint32 color = SDL_MapRGBA(surface->format, 0,
                255 - gradation, gradation, 255);
        SDL_Rect rect = {0, i, width, 1};
        SDL_FillRect(surface, &rect, color);
    }
    return surface;
}

