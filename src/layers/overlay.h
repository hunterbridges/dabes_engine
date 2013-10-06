#ifndef __overlay_h
#define __overlay_h
#include "../graphics/graphics.h"
#include "../entities/entity.h"
#include "../scenes/scene.h"
#include "../math/vpoint.h"

struct Sprite;

typedef enum {
    OVERLAY_ENTITY_EDGE_TL = 0,
    OVERLAY_ENTITY_EDGE_TC = 1,
    OVERLAY_ENTITY_EDGE_TR = 2,
    
    OVERLAY_ENTITY_EDGE_ML = 3,
    OVERLAY_ENTITY_EDGE_MC = 4,
    OVERLAY_ENTITY_EDGE_MR = 5,
    
    OVERLAY_ENTITY_EDGE_BL = 6,
    OVERLAY_ENTITY_EDGE_BC = 7,
    OVERLAY_ENTITY_EDGE_BR = 8
} OverlayEntityEdge;

typedef struct Overlay {
    Scene *scene;
    GfxFont *font;
    DArray *sprites;
    
    Entity *track_entity;
    OverlayEntityEdge track_entity_edge;
    VPoint track_entity_offset;
  
    float alpha;
  
    uint32_t timestamp;
    uint16_t add_index;
    uint64_t ukey;
} Overlay;

Overlay *Overlay_create(Engine *engine, char *font_name, int px_size);
void Overlay_destroy(Overlay *overlay);
void Overlay_update(Overlay *overlay, Engine *engine);
void Overlay_render(Overlay *overlay, Engine *engine);
void Overlay_add_sprite(Overlay *overlay, struct Sprite *sprite);
void Overlay_set_add_index(Overlay *overlay, uint16_t add_index);
int Overlay_cmp(void *a, void *b);

#endif
