#include <math.h>
#include "../graphics/tile_map_parse.h"
#include "../core/engine.h"
#include "scene.h"

Scene *Scene_create(Engine *engine, SceneProto proto) {
    Scene *scene = calloc(1, sizeof(Scene));
    check(scene != NULL, "Couldn't create scene");

    scene->proto = proto;
    scene->_(create)(scene, engine);
    return scene;
error:
    return NULL;
}

void Scene_restart(Scene *scene, Engine *engine) {
    scene->_(stop)(scene, engine);
    scene->_(start)(scene, engine);
}

void Scene_reset_camera(Scene *scene) {
    scene->camera->scale = 1.2;
    scene->camera->rotation_radians = 0;
    scene->camera->track_entity = scene->entities->first->value;
    scene->camera->translation.x = 0;
    scene->camera->translation.y = 0;
    scene->camera->scene_size.w =
        scene->tile_map->cols * scene->tile_map->tile_size.w;
    scene->camera->scene_size.h =
        scene->tile_map->rows * scene->tile_map->tile_size.h;
}

void Scene_draw_debug_grid(Scene *scene, Graphics *graphics) {
#ifndef DABES_IOS
    glUseProgram(0);
    Graphics_reset_projection_matrix(graphics);
    Graphics_reset_modelview_matrix(graphics);

    glDisable(GL_MULTISAMPLE);
    Graphics_translate_modelview_matrix(graphics,
            -SCREEN_WIDTH/2, -SCREEN_HEIGHT/2, 0.f);

    Graphics_reset_projection_matrix(graphics);
    Graphics_scale_projection_matrix(graphics, scene->camera->scale,
            scene->camera->scale, 1);
    Graphics_rotate_projection_matrix(graphics, scene->camera->rotation_radians,
            0, 0, -1);
    glLineWidth(0);
    glColor4f(1.0, 0.0, 0.0, 1.0);
    double grid = scene->world->grid_size;
    double ppm = scene->world->pixels_per_meter;
    int rows = ceil(scene->world->height / scene->world->grid_size);
    int cols = ceil(scene->world->width / scene->world->grid_size);
    int row = 0;
    int col = 0;
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            glBegin(GL_LINES);
            glVertex2f((col * grid) * ppm, (row * grid) * ppm);
            glVertex2f((col * grid + grid) * ppm, (row * grid) * ppm);
            glEnd();

            glBegin(GL_LINES);
            glVertex2f((col * grid + grid) * ppm, (row * grid) * ppm);
            glVertex2f((col * grid + grid) * ppm, (row * grid + grid) * ppm);
            glEnd();

            glBegin(GL_LINES);
            glVertex2f((col * grid + grid) * ppm, (row * grid + grid) * ppm);
            glVertex2f((col * grid) * ppm, (row * grid + grid) * ppm);
            glEnd();

            glBegin(GL_LINES);
            glVertex2f((col * grid) * ppm, (row * grid + grid) * ppm);
            glVertex2f((col * grid) * ppm, (row * grid) * ppm);
            glEnd();
        }
    }
    glEnable(GL_MULTISAMPLE);
    glUseProgram(graphics->shader);
#endif
}

void Scene_set_tile_map(Scene *scene, Engine *engine, TileMap *tile_map) {
    if (scene->tile_map) {
        TileMap_destroy(scene->tile_map);
    }
    scene->tile_map = tile_map;
    Scene_restart(scene, engine);
}

void Scene_load_tile_map(Scene *scene, Engine *engine, char *map_file,
                         int abs_path) {
  const char *map_path = abs_path ? map_file : resource_path(map_file);
  TileMap *map = TileMap_parse((char *)map_path, engine);
  if (map == NULL) return;
  Scene_set_tile_map(scene, engine, map);
}

