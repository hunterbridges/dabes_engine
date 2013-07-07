#include <math.h>
#include "../graphics/tile_map_parse.h"
#include "../core/engine.h"
#include "scene.h"
#include "../graphics/draw_buffer.h"

Scene *Scene_create(Engine *engine, SceneProto proto) {
    Scene *scene = calloc(1, sizeof(Scene));
    check(scene != NULL, "Couldn't create scene");

    scene->name = NULL;
    scene->proto = proto;
    scene->camera = Camera_create(engine->graphics->screen_size.w,
                                  engine->graphics->screen_size.h);
    scene->camera->scene_size = scene->camera->screen_size;

    VVector4 cover_color = {.raw = {0, 0, 0, 0}};
    scene->cover_color = cover_color;

    return scene;
error:
    return NULL;
}

void Scene_destroy(Scene *scene, Engine *engine) {
    check(scene != NULL, "No scene to destroy");

    scene->_(stop)(scene, engine);
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
    scene->_(stop)(scene, engine);
    scene->_(start)(scene, engine);
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
    scene->camera->scene_size =
        TileMap_draw_size(scene->tile_map, scene->pixels_per_meter);
    if(needs_restart) Scene_restart(scene, engine);
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

void Scene_update(Scene *scene, Engine *engine) {
    if (scene->started == 0) return;
    Scene_control(scene, engine);

    if (scene->proto.update != NULL) scene->_(update)(scene, engine);

    Scripting_call_hook(engine->scripting, scene, "main");

    int i = 0;
    for (i = 0; i < DArray_count(scene->entities); i++) {
        Entity *entity = DArray_get(scene->entities, i);
        Entity_update(entity, engine);
    }

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

void Scene_render(Scene *scene, Engine *engine) {
    if (scene->proto.render != NULL) {
        scene->_(render)(scene, engine);
    }
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

            int i = 0;
            for (i = 0; i < DArray_count(scene->entities); i++) {
                Entity *entity = DArray_get(scene->entities, i);
                entity->selected = 0;
            }
            break;


        case kSceneSelectingForCamera:
        default:
            if (!scene->selected_entities) {
                scene->selected_entities = List_create();
            }
            break;
    }
}

int Scene_select_entities_at(Scene *scene, VPoint screen_point) {
    if (scene->selection_mode == kSceneNotSelecting) return 0;

    VPoint graphics_point = Camera_cast_point(scene->camera, screen_point);
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

void Scene_render_entities(Scene *scene, Engine *engine) {
    GfxShader *dshader = Graphics_get_shader(engine->graphics, "decal");
    Graphics_use_shader(engine->graphics, dshader);
    glUniformMatrix4fv(GfxShader_uniforms[UNIFORM_DECAL_PROJECTION_MATRIX], 1,
                       GL_FALSE, engine->graphics->projection_matrix.gl);

    int i = 0;
    for (i = 0; i < DArray_count(scene->entities); i++) {
        Entity *entity = DArray_get(scene->entities, i);
        Entity_render(entity, engine, dshader->draw_buffer);
    }

    DrawBuffer_draw(dshader->draw_buffer);
    DrawBuffer_empty(dshader->draw_buffer);
}

void Scene_render_selected_entities(Scene *scene, Engine *engine) {
    if (scene->selection_mode != kSceneNotSelecting) {
        int i = 0;
        for (i = 0; i < DArray_count(scene->entities); i++) {
            Entity *entity = DArray_get(scene->entities, i);
            if (!entity) continue;

            VRect entity_rect = Entity_real_rect(entity);
            GLfloat color[4] = {1, 1, 1, 0.5};
            if (entity->selected) color[3] = 1.0;
            Graphics_stroke_rect(engine->graphics, entity_rect, color, 2, 0);
        }
    }
}

void Scene_fill(Scene *scene, Engine *engine, VVector4 color) {
    if (color.rgba.a > 0.0) {
        GfxShader *dshader = Graphics_get_shader(engine->graphics, "decal");
        Graphics_use_shader(engine->graphics, dshader);
        Camera screen_cam = {
          .focal = {0, 0},
          .screen_size = scene->camera->screen_size,
          .scale = 1,
          .rotation_radians = 0,
          .margin = scene->camera->margin,
          .translation = {0, 0}
        };
        Graphics_reset_projection_matrix(engine->graphics);
        Graphics_reset_modelview_matrix(engine->graphics);
        Graphics_project_camera(engine->graphics, &screen_cam);
        VRect cover_rect = VRect_from_xywh(-screen_cam.screen_size.w / 2.0,
                                           -screen_cam.screen_size.h / 2.0,
                                           screen_cam.screen_size.w,
                                           screen_cam.screen_size.h);
        glUniformMatrix4fv(GfxShader_uniforms[UNIFORM_DECAL_PROJECTION_MATRIX],
                           1, GL_FALSE, engine->graphics->projection_matrix.gl);
        Graphics_draw_rect(engine->graphics, NULL, cover_rect,
                color.raw, NULL, VPointZero, GfxSizeZero,
                0, 0);
    }
}
