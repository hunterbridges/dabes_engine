#ifndef __sprite_h
#define __sprite_h
#include <lcthw/hashmap.h>
#include "../core/engine.h"
#include "graphics.h"
#include "../core/stepper.h"

typedef struct SpriteFrame {
    VPoint offset;
} SpriteFrame;

typedef struct SpriteAnimation {
    int num_frames;
    int loop_start;
    int current_index;
    int repeats;
    Stepper *stepper;
    int frames[];
} SpriteAnimation;

SpriteAnimation *SpriteAnimation_create(int num_frames, int frames[]);
void SpriteAnimation_destroy(SpriteAnimation *animation);

////////////////////////////////////////////////////////////////////////////////

typedef enum {
    SPRITE_DIR_FACING_RIGHT = 0,
    SPRITE_DIR_FACING_LEFT  = 180
} SpriteDirection;

typedef struct Sprite {
    GfxTexture *texture;
    GfxSize cell_size;
    int rows;
    int cols;
    int num_frames;

    Hashmap *animations;
    SpriteAnimation *current_animation;

    SpriteDirection direction;
    int current_frame;
    SpriteFrame frames[];
} Sprite;

Sprite *Sprite_create(GfxTexture *texture, GfxSize cell_size);
void Sprite_destroy(Sprite *sprite);
void Sprite_update(Sprite *sprite, Engine *engine);

int Sprite_add_animation(Sprite *sprite, SpriteAnimation *animation,
        const char *name);
int Sprite_use_animation(Sprite *sprite, const char *name);

void SpriteAnimation_update(SpriteAnimation *animation, Engine *engine);

#endif
