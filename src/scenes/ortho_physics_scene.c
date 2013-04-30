#include "ortho_physics_scene.h"

int OrthoPhysicsScene_create_world(Scene *scene, Engine *engine);

void OrthoPhysicsScene_start(struct Scene *scene, Engine *engine) {
    if (scene->started) return;
    assert(scene->world == NULL);
    assert(scene->entities == NULL);
    scene->entities = List_create();

    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *entity = GameEntity_create();
        GfxSize cell_size = {32, 32};
        entity->sprite =
            Graphics_sprite_from_image(engine->graphics,
                                       "media/sprites/dumblock.png",
                                       cell_size);
        List_push(scene->entities, entity);
        entity->alpha = (double)i / NUM_BOXES * 1.0;
    }

    Scene_reset_camera(scene);
    scene->bg_texture = Graphics_texture_from_image(engine->graphics,
                                                    "media/sprites/clouds.png");

    OrthoPhysicsScene_create_world(scene, engine);

    GameEntity_assign_controller(scene->entities->first->value,
          engine->input->controllers[0]);

    scene->started = 1;
}

void OrthoPhysicsScene_stop(struct Scene *scene, Engine *UNUSED(engine)) {
    if (!scene->started) return;
    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *entity = current->value;
        GameEntity_destroy(entity);
    }

    List_destroy(scene->entities);
    scene->entities = NULL;

    if (scene->world) {
      scene->world = NULL;
      World_destroy(scene->world);
    }

    scene->started = 0;
}

void OrthoPhysicsScene_cleanup(struct Scene *scene, Engine *engine) {
    check(scene != NULL, "No scene to destroy");

error:
    return;
}

void OrthoPhysicsScene_update(struct Scene *scene, Engine *engine) {
    World_solve(engine->physics, scene->world, scene->tile_map,
                engine->frame_ticks);
    Camera_track(scene->camera);
}

void OrthoPhysicsScene_render(struct Scene *scene, Engine *engine) {
    Graphics *graphics = ((Engine *)engine)->graphics;

    GfxShader *dshader = Graphics_get_shader(graphics, "decal");
    GfxShader *tshader = Graphics_get_shader(graphics, "tilemap");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    Graphics_use_shader(graphics, dshader);

    // TODO: Parallax bg camera
    double bgScale = (scene->camera->scale + 2) / 2;
    VPoint screen_center = {
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
    VRect gfx_rect = VRect_fill_size(bg_size, scene->camera->screen_size);
    Graphics_draw_rect(graphics, gfx_rect, color, scene->bg_texture,
                       VPointZero, scene->bg_texture->size,0);
    Graphics_project_camera(graphics, scene->camera);

    Graphics_use_shader(graphics, tshader);
    TileMap_render(scene->tile_map, graphics,
                   scene->world->pixels_per_meter * scene->world->grid_size);

    Graphics_use_shader(graphics, dshader);
    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *thing = current->value;
        if (thing == NULL) break;
        GameEntity_render(thing, engine);
    }

    // Camera debug
    if (scene->debug_camera)
        Camera_debug(scene->camera, engine->graphics);

    // Draw the grid
    if (scene->draw_grid && scene->world)
        Scene_draw_debug_grid(scene, graphics);
}

void OrthoPhysicsScene_control(struct Scene *scene, Engine *engine) {
    Input *input = engine->input;
    if (input->cam_reset) {
        Scene_reset_camera(scene);
    }
    scene->camera->scale += 0.02 * input->cam_zoom;
    if (scene->camera->scale < 0) scene->camera->scale = 0;

    scene->camera->rotation_radians += 2 * input->cam_rotate * M_PI / 180;

    if (input->cam_debug) scene->debug_camera = !(scene->debug_camera);
    if (input->debug_scene_draw_grid) scene->draw_grid = !(scene->draw_grid);

    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *entity = current->value;
        GameEntity_control(entity, engine);
    }
}

int OrthoPhysicsScene_create_world(Scene *scene, Engine *engine) {
    check_mem(scene);
    check_mem(engine);

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

        entity->physics_shape.fixture = fixture;
        entity->physics_shape.shape_type = GameEntityPhysicsShapeTypeFixture;
        /*
        VRect real_box = Fixture_real_box(fixture);
        fixture->history[0] = real_box;
        fixture->history[1] = real_box;
        */

        WorldGrid_add_fixture(world->grid, fixture);

        i++;
    }

    scene->world = world;

    return 1;
error:
    return 0;
}

SceneProto OrthoPhysicsSceneProto = {
    .start = OrthoPhysicsScene_start,
    .stop = OrthoPhysicsScene_stop,
    .cleanup = OrthoPhysicsScene_cleanup,
    .update = OrthoPhysicsScene_update,
    .render = OrthoPhysicsScene_render,
    .control = OrthoPhysicsScene_control
};
