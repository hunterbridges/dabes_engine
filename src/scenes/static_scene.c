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

void StaticScene_stop(struct Scene *scene, Engine *engine) {
    if (!scene->started) return;

    Scripting_call_hook(engine->scripting, scene, "cleanup");

    DArray_destroy(scene->entities);
    scene->entities = NULL;

    DArray_destroy(scene->overlays);
    scene->overlays = NULL;

    Stepper_reset(engine->physics->stepper);
    scene->started = 0;
}

void StaticScene_start(struct Scene *scene, Engine *engine) {
    if (scene->started) return;
    assert(scene->world == NULL);
    assert(scene->entities == NULL);
    scene->entities = DArray_create(sizeof(Entity *), 8);
    scene->overlays = DArray_create(sizeof(Overlay *), 8);

    if (Scripting_call_hook(engine->scripting, scene, "configure")) {
      scene->started = 1;
      scene->started_at = Engine_get_ticks(engine);
    } else {
      // Need to do a graceful stop since user could have manipulated the scene
      // before the hook hit the error.

      scene->started = 1;
      StaticScene_stop(scene, engine);

      // Now scene->started is 0
    }
}

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

void StaticScene_add_entity(Scene *scene, Engine *engine,
        Entity *entity) {
    assert(entity != NULL);
    assert(scene != NULL);
    entity->scene = scene;
    DArray_push(scene->entities, entity);
    if (scene->space) {
        StaticScene_add_entity_body(scene, engine, entity);
    }
}

Entity *StaticScene_hit_test(Scene *UNUSED(scene), VPoint UNUSED(g_point)) {
    return NULL;
}

SceneProto StaticSceneProto = {
    .start = StaticScene_start,
    .stop = StaticScene_stop,
    .cleanup = StaticScene_cleanup,
    .update = StaticScene_update,
    .render = StaticScene_render,
    .control = StaticScene_control,
    .add_entity = StaticScene_add_entity,
    .hit_test = StaticScene_hit_test
};
