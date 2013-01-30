#ifndef __gfx_helpers_h
#define __gfx_helpers_h
#include "prefix.h"

typedef struct Point {
    float x;
    float y;
} Point;

int initGL(int swidth, int sheight);
int loadSurfaceAsTexture(SDL_Surface *surface);
SDL_Surface *gradient(unsigned int width, unsigned int height);
Point rotate_point(Point point, Point pivot, float angle);

#endif
