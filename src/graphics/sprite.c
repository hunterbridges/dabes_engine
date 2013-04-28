#include <lcthw/bstrlib.h>
#include "sprite.h"

Sprite *Sprite_create(GfxTexture *texture, GfxSize cell_size) {
    check(texture != NULL, "Need texture to build sprite");
    int cols = floor(texture->size.w / cell_size.w);
    int rows = floor(texture->size.h / cell_size.h);
    int num_frames = cols * rows;

    Sprite *sprite = calloc(1, sizeof(Sprite) +
                            num_frames * sizeof(SpriteFrame));
    sprite->cell_size = cell_size;
    sprite->texture = texture;

    sprite->cols = cols;
    sprite->rows = rows;
    sprite->num_frames = num_frames;

    sprite->animations = Hashmap_create(NULL, NULL);

    int i = 0;
    for (i = 0; i < num_frames; i++) {
        int row = i / sprite->cols;
        int col = i % sprite->cols;
        SpriteFrame *frame = &sprite->frames[i];
        frame->offset.x = col * cell_size.w;
        frame->offset.y = row * cell_size.h;
    }

    return sprite;
error:
    return NULL;
}

void Sprite_destroy(Sprite *sprite) {
    check(sprite != NULL, "No sprite to destroy");
    Hashmap_destroy(sprite->animations, NULL);
    free(sprite);
    return;
error:
    return;
}

int Sprite_add_animation(Sprite *sprite, const char *name, int num_frames,
        int frames[]) {
    check(sprite != NULL, "No sprite to add animation to");
    SpriteAnimation *animation = malloc(sizeof(int) +
                                        num_frames * sizeof(int));
    animation->num_frames = num_frames;
    memcpy(animation->frames, frames, num_frames * sizeof(int));

    bstring bname = bfromcstr(name);
    Hashmap_set(sprite->animations, bname, animation);
    bdestroy(bname);
error:
    return 0;
}
