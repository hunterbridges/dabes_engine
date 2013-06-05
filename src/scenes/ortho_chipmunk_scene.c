#include "../core/engine.h"
#include "../audio/audio.h"
#include "../audio/sfx.h"
#include "../entities/body.h"
#include "../entities/body_bindings.h"
#include "ortho_chipmunk_scene.h"

int OrthoChipmunkScene_create_space(Scene *scene, Engine *engine);
void OrthoChipmunkScene_control(struct Scene *scene, Engine *engine);

void OrthoChipmunkScene_start(struct Scene *scene, Engine *engine) {
    if (scene->started) return;
    assert(scene->world == NULL);
    assert(scene->entities == NULL);
    scene->entities = List_create();

    Scene_reset_camera(scene);

    Scripting_call_hook(engine->scripting, scene, "configure");

    // TODO: Track in a script
    if (scene->entities->first) {
        scene->camera->track_entity = scene->entities->first->value;
    }

    OrthoChipmunkScene_create_space(scene, engine);

    scene->started = 1;
}

void OrthoChipmunkScene_stop(struct Scene *scene, Engine *engine) {
    if (!scene->started) return;

    Scripting_call_hook(engine->scripting, scene, "cleanup");

    List_destroy(scene->entities);
    scene->entities = NULL;

    if (scene->space) {
      cpSpaceFree(scene->space);
      scene->space = NULL;
    }

    Stepper_reset(engine->physics->stepper);
    scene->started = 0;
}

void OrthoChipmunkScene_cleanup(struct Scene *scene, Engine *UNUSED(engine)) {
    check(scene != NULL, "No scene to destroy");
error:
    return;
}

void OrthoChipmunkScene_update(struct Scene *scene, Engine *engine) {
    OrthoChipmunkScene_control(scene, engine);

    {LIST_FOREACH(scene->entities, first, next, current) {
        Entity *entity = current->value;
        Scripting_call_hook(engine->scripting, entity, "presolve");
    }}

    int phys_ticks = engine->frame_ticks < 100 ? engine->frame_ticks : 100;
    Stepper_update(engine->physics->stepper, phys_ticks);

    while (Stepper_pop(engine->physics->stepper)) {
      cpSpaceStep(scene->space, engine->physics->stepper->step_skip / 1000.0);
    }

    {LIST_FOREACH(scene->entities, first, next, current) {
        Entity *entity = current->value;
        Entity_update(entity, engine);
    }}

    Camera_track(scene->camera);
}

void OrthoChipmunkScene_render(struct Scene *scene, Engine *engine) {
    Graphics *graphics = ((Engine *)engine)->graphics;

    GfxShader *dshader = Graphics_get_shader(graphics, "decal");
    GfxShader *tshader = Graphics_get_shader(graphics, "tilemap");

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (scene->parallax) Parallax_render(scene->parallax, engine->graphics);

    Graphics_project_camera(graphics, scene->camera);

    Graphics_use_shader(graphics, tshader);
    TileMap_render(scene->tile_map, graphics, scene->pixels_per_meter);

    Graphics_use_shader(graphics, dshader);
    LIST_FOREACH(scene->entities, first, next, current) {
        Entity *thing = current->value;
        if (thing == NULL) break;
        Entity_render(thing, engine);
    }

    // Camera debug
    if (scene->debug_camera)
        Camera_debug(scene->camera, engine->graphics);

    // Draw the grid
    if (scene->draw_grid && scene->world)
        Scene_draw_debug_grid(scene, graphics);
}

void OrthoChipmunkScene_control(struct Scene *scene, Engine *engine) {
    Input *input = engine->input;
    if (input->cam_reset) {
        Scene_reset_camera(scene);
    }
    scene->camera->scale += 0.02 * input->cam_zoom;
    if (scene->camera->scale < 0) scene->camera->scale = 0;

    scene->camera->rotation_radians += 2 * input->cam_rotate * M_PI / 180;

    if (input->cam_debug) scene->debug_camera = !(scene->debug_camera);
    if (input->debug_scene_draw_grid) scene->draw_grid = !(scene->draw_grid);
}

int collision_begin_cb(cpArbiter *arb, cpSpace *UNUSED(space),
        void *UNUSED(data)) {
    cpBody *eBody, *tBody;
    cpArbiterGetBodies(arb, &eBody, &tBody);
    BodyStateData *state_data = cpBodyGetUserData(eBody);
    state_data->on_ground++;

    Engine *engine = (Engine *)state_data->engine;
    Scene *scene = (Scene *)state_data->scene;
    Entity *entity = (Entity *)state_data->entity;
    if (state_data->on_ground > 1) return 1;

    VRect cam_rect = Camera_visible_rect(scene->camera);
    VPoint e_center = Entity_center(entity);
    int onscreen = VRect_contains_point(cam_rect, e_center);
    if (!onscreen) return 1;

    Sfx *clomp = Audio_gen_sfx(engine->audio, "media/sfx/clomp.ogg");
    Sfx_play(clomp);
    return 1;
}

void collision_seperate_cb(cpArbiter *arb, cpSpace *UNUSED(space),
        void *UNUSED(data)) {
    cpBody *eBody, *tBody;
    cpArbiterGetBodies(arb, &eBody, &tBody);
    BodyStateData *state_data = cpBodyGetUserData(eBody);

    state_data->on_ground--;
    if (state_data->on_ground < 0) {
        state_data->on_ground = 0;
    }
}

void OrthoChipmunkScene_add_entity_body(Scene *scene, Engine *engine,
        Entity *entity) {
    assert(entity->body != NULL);
    Body *body = entity->body;

    body->cp_space = scene->space;
    body->state.engine = engine;
    body->state.entity = entity;
    body->state.scene = scene;

    entity->pixels_per_meter = scene->pixels_per_meter;
    cpSpaceAddShape(scene->space, body->cp_shape);
    cpSpaceAddBody(scene->space, body->cp_body);
}

void OrthoChipmunkScene_add_entity(Scene *scene, Engine *engine,
        Entity *entity) {
    assert(entity != NULL);
    assert(scene != NULL);
    List_push(scene->entities, entity);
    if (scene->space) {
        OrthoChipmunkScene_add_entity_body(scene, engine, entity);
    }
}

int OrthoChipmunkScene_create_space(Scene *scene, Engine *engine) {
    check_mem(scene);
    check_mem(engine);

    scene->space = cpSpaceNew();
    cpVect gravity = {0, 9.8};
    cpSpaceSetGravity(scene->space, gravity);
    scene->space->collisionSlop = 0.0;
    scene->space->collisionBias = 0.1;

    cpSpaceAddCollisionHandler(scene->space, OCSCollisionTypeEntity,
                               OCSCollisionTypeTile, collision_begin_cb, NULL,
                               NULL, collision_seperate_cb, NULL);

    int i = 0;
    LIST_FOREACH(scene->entities, first, next, current) {
      Entity *entity = current->value;
      OrthoChipmunkScene_add_entity_body(scene, engine, entity);
      i++;
    }

    if (scene->tile_map) {
        TileMapLayer *base_layer = DArray_get(scene->tile_map->layers, 0);
        cpBody *map_body = cpSpaceGetStaticBody(scene->space);
        float grid_size = scene->tile_map->meters_per_tile;
        int i = 0;
        for (i = 0; i < base_layer->gid_count; i++) {
            uint32_t gid = base_layer->tile_gids[i];
            if (gid == 0) continue;

            // TilesetTile *tile = TileMap_resolve_tile_gid(scene->tile_map, gid);
            int col = i % scene->tile_map->cols;
            int row = i / scene->tile_map->cols;
            float corr = 0.01;
            cpVect tile_verts[4] = {
              { grid_size * col + corr,             grid_size * row + grid_size - corr},
              { grid_size * col + grid_size - corr, grid_size * row + grid_size - corr},
              { grid_size * col + grid_size - corr, grid_size * row + corr},
              { grid_size * col + corr,             grid_size * row + corr},
            };
            cpShape *tile_shape = cpPolyShapeNew(map_body, 4, tile_verts,
                                                cpvzero);
            cpShapeSetFriction(tile_shape, 0.5);
            tile_shape->collision_type = OCSCollisionTypeTile;
            cpSpaceAddStaticShape(scene->space, tile_shape);
        }
    }

    return 1;
error:
    return 0;
}


SceneProto OrthoChipmunkSceneProto = {
    .start = OrthoChipmunkScene_start,
    .stop = OrthoChipmunkScene_stop,
    .cleanup = OrthoChipmunkScene_cleanup,
    .update = OrthoChipmunkScene_update,
    .render = OrthoChipmunkScene_render,
    .control = OrthoChipmunkScene_control,
    .add_entity = OrthoChipmunkScene_add_entity
};
