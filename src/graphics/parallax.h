#ifndef __parallax_h
#define __parallax_h
#include <lcthw/darray.h>
#include "../prefix.h"
#include "../math/vpoint.h"
#include "camera.h"
#include "graphics.h"

typedef struct ParallaxLayer {
    GfxTexture *texture;
    GfxSize texture_size;
    VPoint offset;
    double scale;
    double p_factor;
    double y_wiggle;
    double cascade_top;
    double cascade_bottom;
    float z;
} ParallaxLayer;

ParallaxLayer *ParallaxLayer_create(GfxTexture *tex);
void ParallaxLayer_p_cascade(ParallaxLayer *layer, double top, double bot);

struct Scene;
struct Engine;
typedef struct Parallax {
    DArray *layers;
    Camera *camera;
    GfxSize level_size;
    VVector4 sky_color;
    VVector4 sea_color;
    double y_wiggle;
    double sea_level;
    float bg_z;
    struct Scene *scene;
} Parallax;

Parallax *Parallax_create(struct Engine *engine);
void Parallax_destroy(Parallax *parallax);
int Parallax_add_layer(Parallax *parallax, ParallaxLayer *layer);
void Parallax_render(Parallax *parallax, Graphics *graphics);

#endif
