#include <math.h>
#include "../graphics/tile_map_parse.h"
#include "../core/engine.h"
#include "scene.h"

Scene *Scene_create(Engine *UNUSED(engine), SceneProto proto) {
    Scene *scene = calloc(1, sizeof(Scene));
    check(scene != NULL, "Couldn't create scene");

    scene->name = NULL;
    scene->proto = proto;
    scene->camera = Camera_create(SCREEN_WIDTH, SCREEN_HEIGHT);

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
    if (scene->music) {
        Audio_destroy_music(engine->audio, scene->music);
    }

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

void Scene_reset_camera(Scene *scene) {
    scene->camera->scale = 1;
    scene->camera->rotation_radians = 0;
    scene->camera->translation.x = 0;
    scene->camera->translation.y = 0;
    scene->camera->snap_to_scene = 1;

    if (scene->entities->first) {
        scene->camera->track_entity = scene->entities->first->value;
    } else {
        scene->camera->track_entity = NULL;
    }

    if (scene->tile_map) {
        scene->camera->scene_size =
            TileMap_draw_size(scene->tile_map, scene->pixels_per_meter);
    } else {
        scene->camera->scene_size = scene->camera->screen_size;
    }
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
            Graphics_stroke_rect(graphics, rect, color, 0, 0);
        }
    }
}

void Scene_set_tile_map(Scene *scene, Engine *engine, TileMap *tile_map) {
    int needs_restart = scene->started;

    if (scene->tile_map) {
        TileMap_destroy(scene->tile_map);
    }
    scene->tile_map = tile_map;
    if(needs_restart) Scene_restart(scene, engine);
}

void Scene_load_tile_map(Scene *scene, Engine *engine, char *map_file,
                         int abs_path) {
  const char *map_path = abs_path ? map_file : resource_path(map_file);
  TileMap *map = TileMap_parse((char *)map_path, engine);
  if (map == NULL) return;
  Scene_set_tile_map(scene, engine, map);
}

