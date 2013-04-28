#ifndef __sprite_h
#define __sprite_h
#include <lcthw/hashmap.h>
#include "graphics.h"

typedef struct SpriteFrame {
    VPoint offset;
} SpriteFrame;

typedef struct Sprite {
    GfxTexture *texture;
    GfxSize cell_size;
    int rows;
    int cols;
    int num_frames;
    Hashmap *animations;
    SpriteFrame frames[];
} Sprite;

typedef struct SpriteAnimation {
    int num_frames;
    int frames[];
} SpriteAnimation;

Sprite *Sprite_create(GfxTexture *texture, GfxSize cell_size);
void Sprite_destroy(Sprite *sprite);
int Sprite_add_animation(Sprite *sprite, const char *name, int num_frames,
    int frames[]);

#endif
