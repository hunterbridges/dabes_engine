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
  
    float alpha;
  
    uint16_t z_index;
    uint32_t timestamp;
    uint16_t add_index;

    uint64_t z_key;
} Overlay;

Overlay *Overlay_create(Engine *engine, char *font_name, int px_size);
void Overlay_destroy(Overlay *overlay);
void Overlay_update(Overlay *overlay, Engine *engine);
void Overlay_render(Overlay *overlay, Engine *engine);
void Overlay_add_sprite(Overlay *overlay, struct Sprite *sprite);
void Overlay_set_z_index(Overlay *overlay, uint16_t z_index);
void Overlay_set_add_index(Overlay *overlay, uint16_t add_index);
int Overlay_z_cmp(void *a, void *b);

#endif
