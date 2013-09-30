#include "overlay.h"
#include "../graphics/sprite.h"

Overlay *Overlay_create(Engine *engine, char *font_name, int px_size) {
    Overlay *overlay = calloc(1, sizeof(Overlay));
    check(overlay != NULL, "Couldn't create Overlay");

    if (font_name != NULL) {
        GfxFont *font = GfxFont_create(engine->graphics, font_name, px_size);
        overlay->font = font;
    }

    overlay->alpha = 1;
    overlay->sprites = DArray_create(sizeof(Sprite *), 8);
    overlay->timestamp = (uint32_t)Engine_get_ticks(engine);
    overlay->track_entity_edge = OVERLAY_ENTITY_EDGE_MC;
    Overlay_set_z_index(overlay, 1);

    return overlay;
error:
    return NULL;
}

void Overlay_destroy(Overlay *overlay) {
    check(overlay != NULL, "No Overlay to destroy");

    if (overlay->font) GfxFont_destroy(overlay->font);
    DArray_destroy(overlay->sprites);
    free(overlay);

    return;
error:
    return;
}

void Overlay_add_sprite(Overlay *overlay, Sprite *sprite) {
    check(overlay != NULL, "No overlay to add sprite to");
    check(sprite != NULL, "No sprite to add");
    DArray_push(overlay->sprites, sprite);
    return;
error:
    return;
}

void Overlay_update(Overlay *overlay, Engine *engine) {
    int i = 0;
    for (i = 0; i < DArray_count(overlay->sprites); i++) {
        Sprite *sprite = DArray_get(overlay->sprites, i);
        Sprite_update(sprite, engine);
    }
    Scripting_call_hook(engine->scripting, overlay, "update");
}

VPoint point_from_edge_of_rect(VRect rect, OverlayEntityEdge edge) {
    switch (edge) {
        case OVERLAY_ENTITY_EDGE_TL:
            return rect.tl;

        case OVERLAY_ENTITY_EDGE_TC:
            return VPoint_mid(rect.tl, rect.tr);

        case OVERLAY_ENTITY_EDGE_TR:
            return rect.tr;

        case OVERLAY_ENTITY_EDGE_ML:
            return VPoint_mid(rect.tl, rect.bl);

        case OVERLAY_ENTITY_EDGE_MC:
            return VPoint_mid(rect.tl, rect.br);

        case OVERLAY_ENTITY_EDGE_MR:
            return VPoint_mid(rect.tr, rect.br);

        case OVERLAY_ENTITY_EDGE_BL:
            return rect.bl;

        case OVERLAY_ENTITY_EDGE_BC:
            return VPoint_mid(rect.bl, rect.br);

        case OVERLAY_ENTITY_EDGE_BR:
            return rect.br;
    }
}

void Overlay_render(Overlay *overlay, Engine *engine) {
    Graphics_project_screen_camera(engine->graphics, overlay->scene->camera);
    Graphics_reset_modelview_matrix(engine->graphics);

    if (overlay->track_entity && overlay->track_entity->scene) {
        VPoint world = VPointZero;
        if (overlay->track_entity_edge == OVERLAY_ENTITY_EDGE_MC) {
            world = Entity_center(overlay->track_entity);
        } else {
            VRect bounding = Entity_bounding_rect(overlay->track_entity);
            world = point_from_edge_of_rect(bounding,
                                            overlay->track_entity_edge);
        }
        VPoint screen =
            Camera_project_point(overlay->track_entity->scene->camera,
                                 world, 1, 1);
        Graphics_translate_modelview_matrix(engine->graphics,
                                            screen.x, screen.y, 0);
    }

    Scripting_call_hook(engine->scripting, overlay, "render");
}

void Overlay_refresh_z_key(Overlay *overlay) {
    if (overlay->scene) {
        BSTree_delete(overlay->scene->overlays, &overlay->z_key);
    }
    overlay->z_key = (((uint64_t)overlay->z_index << 48) |
                     ((uint64_t)overlay->timestamp << 16) |
                     overlay->add_index);
    if (overlay->scene) {
        BSTree_set(overlay->scene->overlays, &overlay->z_key, overlay);
    }
}

void Overlay_set_z_index(Overlay *overlay, uint16_t z_index) {
    overlay->z_index = z_index;
    Overlay_refresh_z_key(overlay);
}

void Overlay_set_add_index(Overlay *overlay, uint16_t add_index) {
    overlay->add_index = add_index;
    Overlay_refresh_z_key(overlay);
}

int Overlay_z_cmp(void *a, void *b) {
    uint64_t left = a ? *(uint64_t *)a : 0;
    uint64_t right = b ? *(uint64_t *)b : 0;
    if (left > right) return 1;
    if (left == right) return 0;
    if (left < right) return -1;
    return 0;
}

