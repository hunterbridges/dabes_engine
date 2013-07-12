#include <lcthw/bstrlib.h>
#include "sprite.h"

SpriteAnimation *SpriteAnimation_create(int num_frames, int frames[]) {
    SpriteAnimation *animation = malloc(sizeof(SpriteAnimation) +
                                        num_frames * sizeof(int));
    check(animation != NULL, "Couldn't create SpriteAnimation");
    animation->num_frames = num_frames;
    int i = 0;
    for (i = 0; i < num_frames; i++) {
        animation->frames[i] = frames[i];
    }
    animation->repeats = 1;
    animation->loop_start = frames[0];
    animation->stepper = Stepper_create();
    animation->current_index = 0;
    return animation;
error:
    return NULL;
}

void SpriteAnimation_destroy(SpriteAnimation *animation) {
    check(animation != NULL, "No Animation to destroy");
    Stepper_destroy(animation->stepper);
    free(animation);
error:
    return;
}

////////////////////////////////////////////////////////////////////////////////

Sprite *Sprite_create(GfxTexture *texture, GfxSize cell_size, int padding) {
    check(texture != NULL, "Need texture to build sprite");
    int cols = floor(texture->size.w / cell_size.w);
    int rows = floor(texture->size.h / cell_size.h);
    int num_frames = cols * rows;

    Sprite *sprite = calloc(1, sizeof(Sprite) +
                            num_frames * sizeof(SpriteFrame));
    sprite->cell_size = cell_size;
    sprite->texture = texture;
    sprite->padding = padding;

    sprite->cols = cols;
    sprite->rows = rows;
    sprite->num_frames = num_frames;
    sprite->direction = SPRITE_DIR_FACING_RIGHT;

    sprite->animations = Hashmap_create(NULL, NULL);

    int i = 0;
    for (i = 0; i < num_frames; i++) {
        int row = i / sprite->cols;
        int col = i % sprite->cols;
        SpriteFrame *frame = &sprite->frames[i];
        frame->offset.x = col * (cell_size.w + 2 * padding);
        frame->offset.y = row * (cell_size.h + 2 * padding);
    }

    sprite->current_frame = 0;

    return sprite;
error:
    return NULL;
}

static inline void destroy_anim(void *anim) {
    SpriteAnimation *animation = (SpriteAnimation *)anim;
    SpriteAnimation_destroy(animation);
}

static inline void no_destroy(void *UNUSED(anim)) { }

void Sprite_destroy(Sprite *sprite) {
    check(sprite != NULL, "No sprite to destroy");
    Hashmap_destroy(sprite->animations, no_destroy);
    free(sprite);
    return;
error:
    return;
}

void Sprite_update(Sprite *sprite, Engine *engine) {
    if (!sprite->current_animation) return;
    if (sprite->manual_frames) return;
    SpriteAnimation *cur_anim = sprite->current_animation;

    int idx = -1;
    int i = 0;
    for (i = 0; i < cur_anim->num_frames; i++) {
        if (cur_anim->frames[i] == sprite->current_frame) {
            idx = i;
            break;
        }
    }

    if (idx == -1) {
        sprite->current_frame = cur_anim->frames[0];
        cur_anim->current_index = 0;
        return;
    }
    if (sprite->current_animation->stepper->step_skip == 0) return;

    int new_idx = cur_anim->current_index;
    Stepper_update(cur_anim->stepper, engine->frame_ticks);
    unsigned long steps = Stepper_clear(cur_anim->stepper);
    if (!steps) return;
    new_idx += steps;
    if (new_idx >= cur_anim->num_frames) {
        Scripting_call_hook(engine->scripting, cur_anim, "complete");
        if (cur_anim->repeats) {
            new_idx %= cur_anim->num_frames;
        } else {
            new_idx = cur_anim->num_frames - 1;
        }
    }

    cur_anim->current_index = new_idx;
    sprite->current_frame = cur_anim->frames[new_idx];
}

int Sprite_add_animation(Sprite *sprite, SpriteAnimation *animation,
        const char *name) {
    check(sprite != NULL, "No sprite to add animation to");
    check(animation != NULL, "No animation to add");

    bstring bname = bfromcstr(name);
    Hashmap_set(sprite->animations, bname, animation);
error:
    return 0;
}

int Sprite_use_animation(Sprite *sprite, const char *name) {
    bstring bname = bfromcstr(name);
    SpriteAnimation *animation = Hashmap_get(sprite->animations, bname);
    bdestroy(bname);
    if (animation == NULL) return 0;
    if (animation && sprite->current_animation == animation) return 0;

    Stepper_reset(animation->stepper);
    sprite->current_animation = animation;
    sprite->current_animation->current_index = 0;

    return 1;
}
