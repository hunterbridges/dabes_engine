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
#include "../recorder/recorder.h"
#include "../recorder/chipmunk_recorder.h"
#include "scene.h"

typedef struct OrthoChipmunkSceneCtx {
    List *tile_shapes;
    cpSpace *space;
    DArray *recorders;
} OrthoChipmunkSceneCtx;

int OrthoChipmunkScene_create_space(Scene *scene, Engine *engine);
void OrthoChipmunkScene_control(struct Scene *scene, Engine *engine);

void OrthoChipmunkScene_stop(struct Scene *scene, Engine *engine) {
    if (!scene->started) return;

    Scripting_call_hook(engine->scripting, scene, "cleanup");

    OrthoChipmunkSceneCtx *context = scene->context;

    int i = 0;
    for (i = 0; i < DArray_count(context->recorders); i++) {
        Recorder *recorder = DArray_get(context->recorders, i);
        Recorder_destroy(recorder);
    }
    DArray_destroy(context->recorders);

    LIST_FOREACH(context->tile_shapes, first, next, current) {
        cpShape *shape = current->value;
        cpSpaceRemoveShape(context->space, shape);
        cpShapeFree(shape);
    }
    List_destroy(context->tile_shapes);
    free(context);
    scene->context = NULL;

    DArray_destroy(scene->entities);
    scene->entities = NULL;

    if (scene->space) {
      cpSpaceFree(scene->space);
      scene->space = NULL;
    }

    Stepper_reset(engine->physics->stepper);
    scene->started = 0;
}

void OrthoChipmunkScene_start(struct Scene *scene, Engine *engine) {
    if (scene->started) return;
    assert(scene->world == NULL);
    assert(scene->entities == NULL);
    scene->entities = DArray_create(sizeof(Entity *), 8);

    OrthoChipmunkSceneCtx *context = calloc(1, sizeof(OrthoChipmunkSceneCtx));
    scene->context = context;
    context->tile_shapes = List_create();
    context->recorders = DArray_create(sizeof(Recorder), 4);

    if (Scripting_call_hook(engine->scripting, scene, "configure")) {
      OrthoChipmunkScene_create_space(scene, engine);
      scene->started = 1;
      scene->started_at = Engine_get_ticks(engine);
    } else {
      // Need to do a graceful stop since user could have manipulated the scene
      // before the hook hit the error.

      scene->started = 1;
      OrthoChipmunkScene_stop(scene, engine);

      // Now scene->started is 0
    }
}

void OrthoChipmunkScene_cleanup(struct Scene *scene, Engine *UNUSED(engine)) {
    check(scene != NULL, "No scene to destroy");
error:
    return;
}

void OrthoChipmunkScene_record_recorders(Scene *scene, Engine *UNUSED(engine)) {
    int i = 0;
    OrthoChipmunkSceneCtx *context = scene->context;
    for (i = 0; i < DArray_count(context->recorders); i++) {
        Recorder *recorder = DArray_get(context->recorders, i);
        if (recorder->state == RecorderStateRecording) {
            size_t size;
            void *frame = recorder->_(capture_frame)(recorder, &size);
            Recorder_write_frame(recorder, frame, size);
        }
    }
}

void OrthoChipmunkScene_play_recorders(Scene *scene, Engine *UNUSED(engine)) {
    int i = 0;
    OrthoChipmunkSceneCtx *context = scene->context;
    for (i = 0; i < DArray_count(context->recorders); i++) {
        Recorder *recorder = DArray_get(context->recorders, i);
        if (recorder->state == RecorderStatePlaying) {
            void *frame = Recorder_read_frame(recorder);
            if (frame) {
                recorder->_(apply_frame)(recorder, frame);
            } else {
                // We are done playing.
                Recorder_set_state(recorder, RecorderStateIdle);
            }
        }
    }
}

Recorder *OrthoChipmunkScene_gen_recorder(Scene *scene, Entity *entity) {
    Recorder *recorder = ChipmunkRecorder_create(5, FPS);
    recorder->entity = entity;
    OrthoChipmunkSceneCtx *context = (OrthoChipmunkSceneCtx *)scene->context;
    DArray_push(context->recorders, recorder);
    return recorder;
};

void OrthoChipmunkScene_update(struct Scene *scene, Engine *engine) {
    int i = 0;
    for (i = 0; i < DArray_count(scene->entities); i++) {
        Entity *entity = DArray_get(scene->entities, i);
        Scripting_call_hook(engine->scripting, entity, "presolve");
    }

    int phys_ticks = (int)(engine->frame_ticks < 100 ? engine->frame_ticks : 100);
    Stepper_update(engine->physics->stepper, phys_ticks);

    while (Stepper_pop(engine->physics->stepper)) {
        OrthoChipmunkScene_play_recorders(scene, engine);
        cpSpaceStep(scene->space, engine->physics->stepper->step_skip / 1000.0);
        OrthoChipmunkScene_record_recorders(scene, engine);
    }
}


static void render_shape_iter(cpShape *shape, void *data) {
    cpPolyShape *pshape = (cpPolyShape *)shape;
    OCSIterData *iter_data = (OCSIterData *)data;

    VPoint shape_verts[pshape->numVerts];

    int i = 0;
    cpVect *vert = pshape->verts;
    cpBody *body = cpShapeGetBody(shape);
    cpVect center = {0, 0};
    VPoint vcenter = {0, 0};
    float rot = 0;
    GLfloat color[4] = {1,1,1,1};

    rot = cpBodyGetAngle(body) * 180 / M_PI;
    center = cpBodyGetPos(body);
    vcenter.x = center.x * iter_data->scene->pixels_per_meter;
    vcenter.y = center.y * iter_data->scene->pixels_per_meter;

    if (shape->collision_type == OCSCollisionTypeEntity) {
        color[0] = 0;
        color[1] = 0;
    } else if (shape->collision_type == OCSCollisionTypeSensor) {
        color[1] = 0;
        color[2] = 0;
    } else if (shape->collision_type == OCSCollisionTypeStaticEntity) {
        color[1] = 0;
    }


    for (i = 0; i < pshape->numVerts; i++, vert++) {
        shape_verts[i].x = vert->x * iter_data->scene->pixels_per_meter;
        shape_verts[i].y = vert->y * iter_data->scene->pixels_per_meter;
    }
    Graphics_stroke_poly(iter_data->engine->graphics, pshape->numVerts,
            shape_verts, vcenter, color, 0, rot);
}

void OrthoChipmunkScene_render_physdebug(struct Scene *scene, Engine *engine) {
    Graphics *graphics = ((Engine *)engine)->graphics;
    GfxShader *dshader = Graphics_get_shader(graphics, "decal");
    Graphics_project_camera(graphics, scene->camera);

    Graphics_use_shader(graphics, dshader);
    OCSIterData iter_data = {scene, engine};
    cpSpaceEachShape(scene->space, render_shape_iter, &iter_data);

    // Camera debug
    if (scene->debug_camera)
        Camera_debug(scene->camera, engine->graphics);
}

void OrthoChipmunkScene_render(struct Scene *scene, Engine *engine) {
    if (scene->started == 0) return;
    if (scene->render_mode == kSceneRenderModePhysicsDebug) {
        OrthoChipmunkScene_render_physdebug(scene, engine);
        return;
    }

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

void OrthoChipmunkScene_control(struct Scene *scene, Engine *engine) {
    Input *input = engine->input;
    if (input->phys_render) {
        scene->render_mode =
            (scene->render_mode == kSceneRenderModeNormal ?
             kSceneRenderModePhysicsDebug : kSceneRenderModeNormal);
    }
    if (input->debug_scene_draw_grid) scene->draw_grid = !(scene->draw_grid);
}

int collision_begin_cb(cpArbiter *arb, cpSpace *UNUSED(space),
        void *UNUSED(data)) {
    cpBody *eBody, *tBody;
    cpArbiterGetBodies(arb, &eBody, &tBody);
    BodyStateData *state_data = cpBodyGetUserData(eBody);
    state_data->on_ground++;
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

int sensor_coll_begin_cb(cpArbiter *arb, cpSpace *UNUSED(space),
        void *UNUSED(data)) {
    cpShape *sShape, *tShape;
    cpArbiterGetShapes(arb, &sShape, &tShape);
    Sensor *sensor = cpShapeGetUserData(sShape);
    sensor->on_static++;
    return 1;
}

void sensor_coll_seperate_cb(cpArbiter *arb, cpSpace *UNUSED(space),
        void *UNUSED(data)) {
    cpShape *sShape, *tShape;
    cpArbiterGetShapes(arb, &sShape, &tShape);
    Sensor *sensor = cpShapeGetUserData(sShape);
    sensor->on_static--;
    if (sensor->on_static < 0) {
        sensor->on_static = 0;
    }
}

int sensor_sensor_coll_begin_cb(cpArbiter *arb, cpSpace *UNUSED(space),
        void *UNUSED(data)) {
    cpShape *aShape, *bShape;
    cpArbiterGetShapes(arb, &aShape, &bShape);
    Sensor *sensor_a = cpShapeGetUserData(aShape);
    Sensor *sensor_b = cpShapeGetUserData(bShape);
    Sensor_overlap_sensor(sensor_a, sensor_b);
    Sensor_overlap_sensor(sensor_b, sensor_a);
    return 1;
}

void sensor_sensor_coll_seperate_cb(cpArbiter *arb, cpSpace *UNUSED(space),
        void *UNUSED(data)) {
    cpShape *aShape, *bShape;
    cpArbiterGetShapes(arb, &aShape, &bShape);
    Sensor *sensor_a = cpShapeGetUserData(aShape);
    Sensor *sensor_b = cpShapeGetUserData(bShape);
    Sensor_separate_sensor(sensor_a, sensor_b);
    Sensor_separate_sensor(sensor_b, sensor_a);
}

void OrthoChipmunkScene_add_entity_body(Scene *scene, Engine *engine,
        Entity *entity) {
    assert(entity != NULL);
    assert(entity->body != NULL);
    Body *body = entity->body;

    body->cp_space = scene->space;
    body->state.engine = engine;
    body->state.entity = entity;
    body->state.scene = scene;

    entity->pixels_per_meter = scene->pixels_per_meter;
    cpSpaceAddShape(scene->space, body->cp_shape);

    if (!body->is_rogue) {
        cpSpaceAddBody(scene->space, body->cp_body);
    }

    LIST_FOREACH(body->sensors, first, next, current) {
        Sensor *sensor = current->value;
        cpSpaceAddShape(scene->space, sensor->cp_shape);
        sensor->cp_space = scene->space;
    }
}

void OrthoChipmunkScene_add_entity(Scene *scene, Engine *engine,
        Entity *entity) {
    assert(entity != NULL);
    assert(scene != NULL);
    entity->scene = scene;
    DArray_push(scene->entities, entity);
    if (scene->space) {
        OrthoChipmunkScene_add_entity_body(scene, engine, entity);
    }
}

void hit_test_func(cpShape *shape, cpFloat UNUSED(distance),
        cpVect UNUSED(point), void *data) {
    if (shape->collision_type == OCSCollisionTypeEntity) {
        Entity **top = data;
        cpBody *cpb = shape->body;
        BodyStateData *state = cpb->data;
        if (*top == NULL || state->entity->z_index > (*top)->z_index) {
            *top = state->entity;
        }
    }
}

Entity *OrthoChipmunkScene_hit_test(Scene *scene, VPoint g_point) {
    VPoint world_point = VPoint_scale(g_point, 1.0 / scene->pixels_per_meter);
    Entity *top = NULL;
    cpVect wp = {world_point.x, world_point.y};
    cpSpaceNearestPointQuery(scene->space, wp, 0.0, CP_ALL_LAYERS,
                             CP_NO_GROUP, hit_test_func, &top);
    return top;
}

int OrthoChipmunkScene_create_space(Scene *scene, Engine *engine) {
    check_mem(scene);
    check_mem(engine);

    scene->space = cpSpaceNew();
    OrthoChipmunkSceneCtx *context = scene->context;
    context->space = scene->space;

    cpVect gravity = {0, 9.8};
    cpSpaceSetGravity(scene->space, gravity);
    scene->space->collisionSlop = 0.0;
    scene->space->collisionBias = 0.1;
    cpSpaceSetSleepTimeThreshold(scene->space, 0.4);
    cpSpaceSetIdleSpeedThreshold(scene->space, 1.0);

    cpSpaceAddCollisionHandler(scene->space, OCSCollisionTypeEntity,
                               OCSCollisionTypeTile, collision_begin_cb, NULL,
                               NULL, collision_seperate_cb, NULL);
    cpSpaceAddCollisionHandler(scene->space, OCSCollisionTypeSensor,
                               OCSCollisionTypeTile, sensor_coll_begin_cb, NULL,
                               NULL, sensor_coll_seperate_cb, NULL);
    cpSpaceAddCollisionHandler(scene->space, OCSCollisionTypeSensor,
                               OCSCollisionTypeEntity, sensor_coll_begin_cb,
                               NULL, NULL, sensor_coll_seperate_cb, NULL);
    cpSpaceAddCollisionHandler(scene->space, OCSCollisionTypeSensor,
                               OCSCollisionTypeSensor,
                               sensor_sensor_coll_begin_cb, NULL,
                               NULL, sensor_sensor_coll_seperate_cb, NULL);

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
            float corr = 0;
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
            List_push(context->tile_shapes, tile_shape);
        }
    }

    int i = 0;
    for (i = 0; i < DArray_count(scene->entities); i++) {
        Entity *entity = DArray_get(scene->entities, i);
        OrthoChipmunkScene_add_entity_body(scene, engine, entity);
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
    .add_entity = OrthoChipmunkScene_add_entity,
    .hit_test = OrthoChipmunkScene_hit_test,
    .gen_recorder = OrthoChipmunkScene_gen_recorder
};
