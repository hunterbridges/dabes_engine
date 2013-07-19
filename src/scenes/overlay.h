#ifndef __overlay_h
#define __overlay_h
#include "../graphics/graphics.h"
#include "../entities/entity.h"
#include "../scenes/scene.h"

struct Sprite;

typedef struct Overlay {
    Scene *scene;
    GfxFont *font;
    DArray *sprites;
    Entity *track_entity;
    int z_index;
} Overlay;

Overlay *Overlay_create(Engine *engine, char *font_name, int px_size);
void Overlay_destroy(Overlay *overlay);
void Overlay_update(Overlay *overlay, Engine *engine);
void Overlay_render(Overlay *overlay, Engine *engine);
void Overlay_add_sprite(Overlay *overlay, struct Sprite *sprite);

#endif
