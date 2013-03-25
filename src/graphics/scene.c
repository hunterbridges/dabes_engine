#include <math.h>
#include "../core/engine.h"
#include "scene.h"

void Scene_draw_debug_grid(Scene *scene, Graphics *graphics);

int Scene_init(void *self) {
    check_mem(self);

    Scene *scene = (Scene *)self;
    scene->music = Music_load("media/music/Tower.wav");
    Music_play(scene->music);
    scene->world = NULL;
    scene->entities = List_create();

    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *entity = NEW(GameEntity, "A thing");
        entity->texture = load_image_as_texture("media/sprites/dumblock.png");
        List_push(scene->entities, entity);
        entity->alpha = (double)i / NUM_BOXES * 1.0;
    }

    scene->camera = Camera_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    Scene_reset_camera(scene);

    scene->bg_texture = load_image_as_texture("media/sprites/clouds.png");

    scene->draw_grid = 0;

    return 1;
error:
    return 0;
}

void Scene_destroy(void *self) {
    check_mem(self);
    Scene *scene = (Scene *)self;

    Music_destroy(scene->music);
    Camera_destroy(scene->camera);

    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *thing = current->value;
        thing->_(destroy)(thing);
    }

    List_destroy(scene->entities);
    if (scene->world) scene->world->_(destroy)(scene->world);
    glDeleteTextures(1, &scene->bg_texture);
    Object_destroy(self);
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
    Graphics_draw_rect(graphics, gfx_rect, color, scene->bg_texture, 0);
    Graphics_project_camera(graphics, scene->camera);
  
    GLfloat fl_color[4] = {0.65, 0.65, 0.65, 1};
    GfxRect ceil_rect =
        GfxRect_from_xywh(-SCREEN_WIDTH, -SCREEN_HEIGHT, 3 * SCREEN_WIDTH,
                          SCREEN_HEIGHT);
    Graphics_draw_rect(graphics, ceil_rect, fl_color, 0, 0);
  
    GfxRect floor_rect =
        GfxRect_from_xywh(-SCREEN_WIDTH, SCREEN_HEIGHT, 3 * SCREEN_WIDTH,
                          SCREEN_HEIGHT);
    Graphics_draw_rect(graphics, floor_rect, fl_color, 0, 0);
  
    GfxRect lwall_rect =
        GfxRect_from_xywh(SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    Graphics_draw_rect(graphics, lwall_rect, fl_color, 0, 0);
  
    GfxRect rwall_rect =
        GfxRect_from_xywh(-SCREEN_WIDTH, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    Graphics_draw_rect(graphics, rwall_rect, fl_color, 0, 0);
  
    // Draw the stuff
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

    World *world = NEW(World, "The world");
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

Object SceneProto = {
   .init = Scene_init,
   .destroy = Scene_destroy
};

