#ifndef __parallax_h
#define __parallax_h
#include <lcthw/darray.h>
#include "../prefix.h"
#include "../math/vpoint.h"
#include "camera.h"
#include "graphics.h"

typedef struct ParallaxLayer {
    GfxTexture *texture;
    VPoint offset;
    double scale;
    double p_factor;
    double y_wiggle;
} ParallaxLayer;

ParallaxLayer *ParallaxLayer_create(GfxTexture *tex);

typedef struct Parallax {
    DArray *layers;
    Camera *camera;
    GfxSize level_size;
    GfxUVertex sky_color;
    GfxUVertex sea_color;
    double y_wiggle;
    double sea_level;
} Parallax;

Parallax *Parallax_create();
void Parallax_destroy(Parallax *parallax);
int Parallax_add_layer(Parallax *parallax, ParallaxLayer *layer);
void Parallax_render(Parallax *parallax, Graphics *graphics);

#endif
