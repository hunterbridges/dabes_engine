#include <lcthw/darray_algos.h>
#include <math.h>
#include "../graphics/tile_map_parse.h"
#include "../core/engine.h"
#include "scene.h"
#include "../graphics/draw_buffer.h"
#include "../layers/overlay.h"

typedef struct SceneTraverseCtx {
    Scene *scene;
    Engine *engine;
    DrawBuffer *draw_buffer;
} SceneTraverseCtx;

Scene *Scene_create(Engine *engine, SceneProto proto) {
    Scene *scene = calloc(1, sizeof(Scene));
    check(scene != NULL, "Couldn't create scene");

    scene->name = NULL;
    scene->proto = proto;
    scene->camera = Camera_create(engine->graphics->screen_size.w,
                                  engine->graphics->screen_size.h);
    scene->canvas = NULL;

    VVector4 cover_color = {.raw = {0, 0, 0, 0}};
    scene->cover_color = cover_color;

    scene->gravity = VPointZero;
    return scene;
error:
    return NULL;
}

void Scene_destroy(Scene *scene, Engine *engine) {
    check(scene != NULL, "No scene to destroy");

    if (scene->started) {
        scene->_(stop)(scene, engine);
    }
    scene->_(cleanup)(scene, engine);

    free(scene->name);

    Camera_destroy(scene->camera);

    if (scene->tile_map) {
        TileMap_destroy(scene->tile_map);
    }

    free(scene);
  return;
error:
    return;
}

void Scene_restart(Scene *scene, Engine *engine) {
    Scene_stop(scene, engine);
    Scene_start(scene, engine);
}

void Scene_reset_camera(Scene *scene, Engine *engine) {
    Scripting_call_hook(engine->scripting, scene, "reset_camera");
}

void Scene_draw_debug_grid(Scene *scene, Graphics *graphics) {
    Graphics_reset_projection_matrix(graphics);
    Graphics_reset_modelview_matrix(graphics);

    Graphics_translate_modelview_matrix(graphics,
            -SCREEN_WIDTH/2, -SCREEN_HEIGHT/2, 0.f);

    Graphics_reset_projection_matrix(graphics);
    Graphics_project_camera(graphics, scene->camera);
    GLfloat color[4] = {0.5, 0.5, 0.5, 0.5};
    double grid = scene->world->grid_size;
    double ppm = scene->world->pixels_per_meter;
    int rows = ceil(scene->world->height / scene->world->grid_size);
    int cols = ceil(scene->world->width / scene->world->grid_size);
    int row = 0;
    int col = 0;
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            VRect rect = VRect_from_xywh(col * grid * ppm,
                                         row * grid * ppm,
                                         grid * ppm,
                                         grid * ppm);
            Graphics_stroke_rect(graphics, rect, color, 1, 0);
        }
    }
}

void Scene_set_tile_map(Scene *scene, Engine *engine, TileMap *tile_map) {
    int needs_restart = scene->started;

    if (scene->tile_map) {
        TileMap_destroy(scene->tile_map);
    }
    scene->tile_map = tile_map;
    Camera_set_scene_size(scene->camera,
        TileMap_draw_size(scene->tile_map, scene->pixels_per_meter));
    if(needs_restart) Scene_restart(scene, engine);
}

void Scene_set_music(Scene *scene, Music *music) {
    scene->music = music;
}

void Scene_load_tile_map(Scene *scene, Engine *engine, char *map_file,
                         int abs_path, float meters_per_tile) {
  TileMap *map = NULL;
  if (abs_path) {
    char *map_path = map_file;
    map = TileMap_parse(map_path, engine);
  } else {
    char *map_path = engine->project_path(map_file);
    map = TileMap_parse(map_path, engine);
    free(map_path);
  }

  if (map == NULL) return;
  map->meters_per_tile = meters_per_tile;
  Scene_set_tile_map(scene, engine, map);
}

static inline int entity_update_traverse_cb(BSTreeNode *node, void *context) {
    SceneTraverseCtx *ctx = (SceneTraverseCtx *)context;
    Entity *entity = node->data;
    Entity_update(entity, ctx->engine);
    return 0;
}

static inline int overlay_update_traverse_cb(BSTreeNode *node, void *context) {
    SceneTraverseCtx *ctx = (SceneTraverseCtx *)context;
    Overlay *overlay = node->data;
    Overlay_update(overlay, ctx->engine);
    return 0;
}

void Scene_update(Scene *scene, Engine *engine) {
    if (scene->started == 0) return;
    Scene_control(scene, engine);

    if (scene->canvas) {
        Canvas_update(scene->canvas, engine);
    }

    int i = 0;
    for (i = 0; i < INPUT_NUM_CONTROLLERS; i++) {
        Scripting_call_hook(engine->scripting, engine->input->controllers[i],
                            "update");
    }

    if (scene->proto.update != NULL) scene->_(update)(scene, engine);

    Scripting_call_hook(engine->scripting, scene, "main");

    SceneTraverseCtx ctx = {.scene = scene, .engine = engine};
    BSTree_traverse(scene->entities, entity_update_traverse_cb, &ctx);
    BSTree_traverse(scene->overlays, overlay_update_traverse_cb, &ctx);

    Camera_track(scene->camera);
}

void Scene_control(Scene *scene, Engine *engine) {
    Input *input = engine->input;
    if (input->cam_reset) {
        Scene_reset_camera(scene, engine);
    }
    scene->camera->scale += 0.02 * input->cam_zoom;
    if (scene->camera->scale < 0) scene->camera->scale = 0;

    scene->camera->rotation_radians += 2 * input->cam_rotate * M_PI / 180;

    VPointRel focal_rel = VPoint_rel(input->cam_focal_pan, VPointZero);
    if (focal_rel != VPointRelWithin) {
      Camera_track_entities(scene->camera, 0, NULL);
    }

    float pan_scale = 0;
    if (!fequal(scene->camera->scale, 0.0)) {
        pan_scale = 10.f / scene->camera->scale;
    }
    VPoint focal_pan = input->cam_focal_pan;
    focal_pan.y *= -1;
    VPoint translate_pan = input->cam_translate_pan;
    translate_pan.y *= -1;
    scene->camera->focal =
          VPoint_add(scene->camera->focal,
                     VPoint_scale(focal_pan, pan_scale));
    scene->camera->translation =
        VPoint_add(scene->camera->translation,
                   VPoint_scale(translate_pan, pan_scale));

    if (input->cam_debug) scene->debug_camera = !(scene->debug_camera);
    if (scene->proto.control != NULL) scene->_(control)(scene, engine);
}

void Scene_start(Scene *scene, Engine *engine) {
    check(scene != NULL, "No scene to start");
    check(engine != NULL, "Scene requires engine to start");
    if (scene->started) return;
    assert(scene->entities == NULL);
    scene->entities = BSTree_create(Entity_z_cmp);
    scene->overlays = BSTree_create(Overlay_z_cmp);

    if (scene->proto.start) scene->_(start)(scene, engine);

    if (Scripting_call_hook(engine->scripting, scene, "started")) {
        if (scene->proto.start_success_cb) {
            scene->_(start_success_cb)(scene, engine);
        }
        scene->started = 1;
        scene->started_at = Engine_get_ticks(engine);
    } else {
        // Need to do a graceful stop since user could have manipulated the scene
        // before the hook hit the error.

        scene->started = 1;
        Scene_stop(scene, engine);

        // Now scene->started is 0
    }
    return;
error:
    return;
}

void Scene_stop(Scene *scene, Engine *engine) {
    check(scene != NULL, "No scene to stop");
    check(engine != NULL, "Scene requires engine to stop");

    Scripting_call_hook(engine->scripting, scene, "cleanup");

    if (scene->proto.stop) scene->_(stop)(scene, engine);
    BSTree_destroy(scene->entities);
    scene->entities = NULL;

    BSTree_destroy(scene->overlays);
    scene->overlays = NULL;

    scene->canvas = NULL;

    scene->started = 0;
    return;
error:
    return;
}

void Scene_render(Scene *scene, Engine *engine) {
    if (scene->proto.render != NULL) {
        scene->_(render)(scene, engine);
    }
}

static inline int entity_deselect_traverse_cb(BSTreeNode *node,
        void *UNUSED(context)) {
    Entity *entity = node->data;
    entity->selected = 0;
    return 0;
}

void Scene_set_selection_mode(Scene *scene, SceneEntitySelectionMode mode) {
    if (scene->selection_mode == mode) return;

    scene->selection_mode = mode;
    switch (mode) {
        case kSceneNotSelecting:
            if (scene->selected_entities) {
                List_destroy(scene->selected_entities);
                scene->selected_entities = NULL;
            }

            BSTree_traverse(scene->entities, entity_deselect_traverse_cb, NULL);
            break;


        case kSceneSelectingForCamera:
        case kSceneSelectingForRecorder:
        default:
            if (!scene->selected_entities) {
                scene->selected_entities = List_create();
            }
            break;
    }
}

int Scene_select_entities_at(Scene *scene, VPoint screen_point) {
    if (scene->selection_mode == kSceneNotSelecting) return 0;

    VPoint graphics_point = Camera_cast_point(scene->camera, screen_point, 1);
    Entity *hit = NULL;
    if (scene->proto.hit_test) {
        hit = scene->_(hit_test)(scene, graphics_point);
    }

    if (hit) {
        if (hit->selected) {
            List_remove_value(scene->selected_entities, hit);
            hit->selected = 0;
        } else {
            List_push(scene->selected_entities, hit);
            hit->selected = 1;
        }
    }

    return hit && hit->selected;
}

#pragma mark - Rendering

static inline int entity_render_traverse_cb(BSTreeNode *node, void *context) {
    SceneTraverseCtx *ctx = (SceneTraverseCtx *)context;
    Entity *entity = node->data;
    Entity_render(entity, ctx->engine, ctx->draw_buffer);
    return 0;
}

void Scene_render_entities(Scene *scene, Engine *engine) {
    GfxShader *dshader = Graphics_get_shader(engine->graphics, "decal");
    Graphics_use_shader(engine->graphics, dshader);
    Graphics_uniformMatrix4fv(engine->graphics,
                              UNIFORM_DECAL_PROJECTION_MATRIX,
                              engine->graphics->projection_matrix.gl,
                              GL_FALSE);

    SceneTraverseCtx ctx = {.engine = engine,
        .draw_buffer = dshader->draw_buffer};
    BSTree_traverse(scene->entities, entity_render_traverse_cb, &ctx);

    DrawBuffer_draw(dshader->draw_buffer, engine->graphics);
    DrawBuffer_empty(dshader->draw_buffer);
}

static inline int entity_render_sel_traverse_cb(BSTreeNode *node,
        void *context) {
    SceneTraverseCtx *ctx = (SceneTraverseCtx *)context;
    Entity *entity = node->data;

    VRect entity_rect = Entity_real_rect(entity);
    GLfloat color[4] = {1, 1, 1, 0.5};
    if (entity->selected) color[3] = 1.0;
    Graphics_stroke_rect(ctx->engine->graphics, entity_rect, color, 2, 0);
    return 0;
}


void Scene_render_selected_entities(Scene *scene, Engine *engine) {
    if (scene->selection_mode != kSceneNotSelecting) {
        SceneTraverseCtx ctx = {.engine = engine};
        BSTree_traverse(scene->entities, entity_render_sel_traverse_cb, &ctx);
    }
}

void Scene_project_screen(Scene *scene, Engine *engine) {
    Graphics_project_screen_camera(engine->graphics, scene->camera);
}

void Scene_fill(Scene *scene, Engine *engine, VVector4 color) {
    if (color.a > 0.0) {
        GfxShader *dshader = Graphics_get_shader(engine->graphics, "decal");
        Graphics_use_shader(engine->graphics, dshader);
        Graphics_reset_modelview_matrix(engine->graphics);
        Scene_project_screen(scene, engine);
        VRect cover_rect = VRect_from_xywh(-scene->camera->screen_size.w / 2.0,
                                           -scene->camera->screen_size.h / 2.0,
                                           scene->camera->screen_size.w,
                                           scene->camera->screen_size.h);
        Graphics_uniformMatrix4fv(engine->graphics,
                                  UNIFORM_DECAL_PROJECTION_MATRIX,
                                  engine->graphics->projection_matrix.gl,
                                  GL_FALSE);
        Graphics_draw_rect(engine->graphics, NULL, cover_rect,
                color.raw, NULL, VPointZero, GfxSizeZero,
                0, 0, 1);
    }
}

static inline int overlay_render_traverse_cb(BSTreeNode *node, void *context) {
    SceneTraverseCtx *ctx = (SceneTraverseCtx *)context;
    Overlay *overlay = node->data;
    Overlay_render(overlay, ctx->engine);
    return 0;
}

void Scene_render_overlays(Scene *scene, Engine *engine) {
    SceneTraverseCtx ctx = {.engine = engine};
    BSTree_traverse(scene->overlays, overlay_render_traverse_cb, &ctx);
}

void Scene_add_entity(Scene *scene, Engine *engine, struct Entity *entity) {
    assert(entity != NULL);
    assert(scene != NULL);
    entity->pixels_per_meter = scene->pixels_per_meter;
    Entity_set_add_index(entity, scene->entity_count++);
    BSTree_set(scene->entities, &entity->z_key, entity);
    entity->scene = scene;
    if (scene->proto.add_entity_cb) {
        scene->_(add_entity_cb)(scene, engine, entity);
    }
}

void Scene_remove_entity(Scene *scene, Engine *engine,
                         struct Entity *entity) {
    assert(entity != NULL);
    assert(scene != NULL);
    BSTree_delete(scene->entities, &entity->z_key);
    if (scene->proto.remove_entity_cb) {
        scene->_(remove_entity_cb)(scene, engine, entity);
    }
}

void Scene_add_overlay(Scene *scene, Overlay *overlay) {
    assert(overlay != NULL);
    assert(scene != NULL);
    Overlay_set_add_index(overlay, scene->overlay_count++);
    BSTree_set(scene->overlays, &overlay->z_key, overlay);
    overlay->scene = scene;
}

void Scene_remove_overlay(Scene *scene, Overlay *overlay) {
    assert(overlay != NULL);
    assert(scene != NULL);
    BSTree_delete(scene->overlays, &overlay->z_key);
}
