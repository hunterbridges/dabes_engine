#ifndef __graphics_h
#define __graphics_h
#include "prefix.h"

int initGL(int swidth, int sheight);
int loadSurfaceAsTexture(SDL_Surface *surface);
SDL_Surface *gradient(unsigned int width, unsigned int height);

#endif
