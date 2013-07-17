#include "overlay.h"

Overlay *Overlay_create(Engine *engine, char *font_name, int px_size) {
    Overlay *overlay = calloc(1, sizeof(Overlay));
    check(overlay != NULL, "Couldn't create Overlay");

    if (font_name != NULL) {
        GfxFont *font = GfxFont_create(engine->graphics, font_name, px_size);
        overlay->font = font;
    }

    return overlay;
error:
    return NULL;
}

void Overlay_destroy(Overlay *overlay) {
    check(overlay != NULL, "No Overlay to destroy");

    if (overlay->font) GfxFont_destroy(overlay->font);
    free(overlay);

    return;
error:
    return;
}

void Overlay_update(Overlay *overlay, Engine *engine) {
    Scripting_call_hook(engine->scripting, overlay, "update");
}

void Overlay_render(Overlay *overlay, Engine *engine) {
    Graphics_project_screen_camera(engine->graphics, overlay->scene->camera);
    Graphics_reset_modelview_matrix(engine->graphics);

    Scripting_call_hook(engine->scripting, overlay, "render");
}
