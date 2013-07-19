#include "overlay.h"
#include "../graphics/sprite.h"

Overlay *Overlay_create(Engine *engine, char *font_name, int px_size) {
    Overlay *overlay = calloc(1, sizeof(Overlay));
    check(overlay != NULL, "Couldn't create Overlay");

    if (font_name != NULL) {
        GfxFont *font = GfxFont_create(engine->graphics, font_name, px_size);
        overlay->font = font;
    }

    overlay->sprites = DArray_create(sizeof(Sprite *), 8);

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

void Overlay_render(Overlay *overlay, Engine *engine) {
    Graphics_project_screen_camera(engine->graphics, overlay->scene->camera);
    Graphics_reset_modelview_matrix(engine->graphics);

    Scripting_call_hook(engine->scripting, overlay, "render");
}
