#include <math.h>
#include "tile_map_parse.h"
#include "../core/engine.h"
#include "scene.h"

void Scene_draw_debug_grid(Scene *scene, Graphics *graphics);

void Scene_start(Scene *scene, Engine *engine) {
    if (scene->started) return;
    assert(scene->world == NULL);
    assert(scene->entities == NULL);
    scene->entities = List_create();

    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *entity = NEW(GameEntity, "A thing");
        entity->texture =
            Graphics_texture_from_image(engine->graphics,
                                        "media/sprites/dumblock.png");
        List_push(scene->entities, entity);
        entity->alpha = (double)i / NUM_BOXES * 1.0;
    }

    Scene_reset_camera(scene);
    scene->bg_texture = Graphics_texture_from_image(engine->graphics,
                                                    "media/sprites/clouds.png");
    scene->draw_grid = 0;
  
    Scene_create_world(scene, engine->physics);
  
    GameEntity_assign_controller(scene->entities->first->value,
          engine->input->controllers[0]);
  
  scene->started = 1;
}

Scene *Scene_create(Engine *engine) {
    Scene *scene = malloc(sizeof(Scene));
    check(scene != NULL, "Couldn't create scene");
  
    scene->music = Music_load("media/music/Climb.aif",
                              "media/music/Climb_Loop.aif");
    Music_play(scene->music);
    scene->camera = Camera_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    scene->tile_map = NULL;
    scene->world = NULL;
    scene->entities = NULL;
    scene->started = 0;
    Scene_load_tile_map(scene, engine, "media/tilemaps/reasonable.tmx", 0);
    Scene_start(scene, engine);
  
    return scene;
error:
    return NULL;
}

void Scene_stop(Scene *scene) {
    if (!scene->started) return;
    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *thing = current->value;
        thing->_(destroy)(thing);
    }

    List_destroy(scene->entities);
    scene->entities = NULL;
  
    if (scene->world) {
      scene->world = NULL;
      World_destroy(scene->world);
    }
  
    scene->started = 0;
}

void Scene_restart(Scene *scene, Engine *engine) {
    Scene_stop(scene);
    Scene_start(scene, engine);
}

void Scene_destroy(Scene *scene) {
    check(scene != NULL, "No scene to destroy");
  
    Scene_stop(scene);

    Music_destroy(scene->music);
    Camera_destroy(scene->camera);

    if (scene->tile_map) {
        TileMap_destroy(scene->tile_map);
    }
  
    free(scene);
error:
    return;
}

void Scene_update(Scene *scene, void *engine) {
    Camera_track(scene->camera);
}

void Scene_render(Scene *scene, void *engine) {
    Graphics *graphics = ((Engine *)engine)->graphics;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // TODO: Parallax bg camera
    double bgScale = (scene->camera->scale + 2) / 2;
    GfxPoint screen_center = {
      .x = scene->camera->screen_size.w / 2.0,
      .y = scene->camera->screen_size.h / 2.0
    };
    Camera bgCamera = {
        .focal = screen_center,
        .screen_size = scene->camera->screen_size,
        .scale = bgScale,
        .rotation_radians = 0
    };
    Graphics_project_camera(graphics, &bgCamera);

    GLfloat color[4] = {0, 0, 1, 1};
    GfxSize bg_size = {.w = 512, .h = 384};
    GfxRect gfx_rect = GfxRect_fill_size(bg_size, scene->camera->screen_size);
    Graphics_draw_rect(graphics, gfx_rect, color, scene->bg_texture,
                       GFX_POINT_ZERO, scene->bg_texture->size,0);
    Graphics_project_camera(graphics, scene->camera);
  
    TileMap_render(scene->tile_map, graphics,
                   scene->world->pixels_per_meter * scene->world->grid_size);
  
    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *thing = current->value;
        if (thing == NULL) break;
        GameEntity_render(thing, engine);
    }

    // Draw the grid
    if (!scene->draw_grid || !scene->world) return;
    Scene_draw_debug_grid(scene, graphics);
}

World *Scene_create_world(Scene *scene, Physics *physics) {
    check_mem(scene);
    check_mem(physics);

    assert(scene->tile_map != NULL);
    World *world =
        World_create(scene->tile_map->cols, scene->tile_map->rows);
    check_mem(world);

    int i = 0;
    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *entity = current->value;
        if (entity == NULL) break;

        Fixture *fixture = World_create_fixture(world);
        fixture->width =  1;
        fixture->height = fixture->width;// + fixture->width * 2 * i / NUM_BOXES;
        fixture->center.x = (world->width - (NUM_BOXES - i) * fixture->width * 4 + 2 * (NUM_BOXES + 1) * fixture->width) / 2;
        fixture->center.y = 1;
        fixture->time_scale = 1;
        fixture->rotation_radians = M_PI / 16 * (i % 8);
        Fixture_set_mass(fixture, 100);

        entity->fixture = fixture;
        /*
        PhysBox real_box = Fixture_real_box(fixture);
        fixture->history[0] = real_box;
        fixture->history[1] = real_box;
        */

        WorldGrid_add_fixture(world->grid, fixture);

        i++;
    }

    scene->world = world;
  
    return world;
error:
    return NULL;
}

void Scene_reset_camera(Scene *scene) {
    scene->camera->scale = 1;
    scene->camera->rotation_radians = 0;
    scene->camera->track_entity = scene->entities->first->value;
    scene->camera->translation.x = 0;
    scene->camera->translation.y = 0;
}

void Scene_control(Scene *scene, Input *input) {
    if (input->cam_reset) {
        Scene_reset_camera(scene);
    }
    scene->camera->scale += 0.02 * input->cam_zoom;
    if (scene->camera->scale < 0) scene->camera->scale = 0;

    scene->camera->rotation_radians += 2 * input->cam_rotate;

    if (input->debug_scene_draw_grid) scene->draw_grid = !(scene->draw_grid);

    double volume = scene->camera->scale;
    Music_set_volume(scene->music, volume);

    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *entity = current->value;
        GameEntity_control(entity, input);
    }
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

