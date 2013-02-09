#ifndef __gfx_helpers_h
#define __gfx_helpers_h
#include "prefix.h"

int init_GL(int swidth, int sheight);
GLuint load_surface_as_texture(SDL_Surface *surface);
GLuint load_image_as_texture(char *image_name);
SDL_Surface *gradient(unsigned int width, unsigned int height);

#endif
