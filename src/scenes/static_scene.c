#include <lcthw/darray_algos.h>
#include "../core/engine.h"
#include "../audio/audio.h"
#include "../audio/sfx.h"
#include "../entities/body.h"
#include "../entities/body_bindings.h"
#include "../entities/sensor.h"
#include "../graphics/draw_buffer.h"
#include "ortho_chipmunk_scene.h"
#include "../entities/entity.h"
#include "scene.h"
#include "overlay.h"

void StaticScene_cleanup(struct Scene *scene, Engine *UNUSED(engine)) {
    check(scene != NULL, "No scene to destroy");
error:
    return;
}

void StaticScene_control(struct Scene *UNUSED(scene),
        Engine *UNUSED(engine)) { }

void StaticScene_update(struct Scene *UNUSED(scene), Engine *UNUSED(engine)) { }

void StaticScene_render(struct Scene *scene, Engine *engine) {
    if (scene->started == 0) return;

    Graphics *graphics = ((Engine *)engine)->graphics;

    GfxShader *dshader = Graphics_get_shader(graphics, "decal");
    GfxShader *tshader = Graphics_get_shader(graphics, "tilemap");

    Scene_fill(scene, engine, scene->bg_color);

    if (scene->parallax) {
        Parallax_render(scene->parallax, engine->graphics);
    }

    Graphics_project_camera(graphics, scene->camera);
    if (scene->tile_map) {
        Graphics_use_shader(graphics, tshader);
        TileMap_render(scene->tile_map, graphics, scene->pixels_per_meter);
    }

    Scene_render_entities(scene, engine);
    Scene_render_overlays(scene, engine);
    Graphics_use_shader(graphics, dshader);
    Scene_render_selected_entities(scene, engine);
    Scene_fill(scene, engine, scene->cover_color);
    if (scene->debug_camera) {
        Camera_debug(scene->camera, engine->graphics);
    }
    if (scene->draw_grid && scene->world) {
        Scene_draw_debug_grid(scene, graphics);
    }
}

void StaticScene_add_entity_body(Scene *UNUSED(scene), Engine *UNUSED(engine),
        Entity *UNUSED(entity)) { }

Entity *StaticScene_hit_test(Scene *UNUSED(scene), VPoint UNUSED(g_point)) {
    return NULL;
}

VPoint StaticScene_get_gravity(struct Scene *scene) {
    check(scene != NULL, "No scene to get gravity from");
    return scene->gravity;
error:
    return VPointZero;
}

void StaticScene_set_gravity(struct Scene *scene, VPoint gravity) {
    check(scene != NULL, "No scene to set gravity on");
    scene->gravity = gravity;
error:
    return;
}

SceneProto StaticSceneProto = {
    .start = NULL,
    .start_success_cb = NULL,
    .stop = NULL,
    .cleanup = StaticScene_cleanup,
    .update = StaticScene_update,
    .render = StaticScene_render,
    .control = StaticScene_control,
    .hit_test = StaticScene_hit_test,
    .add_entity_cb = NULL,
    .remove_entity_cb = NULL,
    .get_gravity = StaticScene_get_gravity,
    .set_gravity = StaticScene_set_gravity
};
