#include <lcthw/darray_algos.h>
#include <lcthw/bstree.h>
#include "../core/engine.h"
#include "../audio/audio.h"
#include "../audio/sfx.h"
#include "../entities/body.h"
#include "../entities/body_bindings.h"
#include "../entities/entity.h"
#include "../entities/sensor.h"
#include "../graphics/draw_buffer.h"
#include "../layers/overlay.h"
#include "../math/vpolygon.h"
#include "../recorder/recorder.h"
#include "../recorder/chipmunk_recorder.h"
#include "chipmunk_scene.h"
#include "scene.h"

typedef struct ChipmunkSceneTraverseCtx {
    Scene *scene;
    Engine *engine;
} OCSTraverseCtx;

typedef struct ChipmunkSceneCtx {
    Scene *scene;
    Engine *engine;

    List *tile_shapes;
    cpSpace *space;
    BSTree *recorders;
} ChipmunkSceneCtx;

int ChipmunkScene_create_space(Scene *scene, Engine *engine);
void ChipmunkScene_control(struct Scene *scene, Engine *engine);

int recorder_destroy_cb(BSTreeNode *node, void *UNUSED(context)) {
    Recorder *recorder = node->data;
    Recorder_destroy(recorder);
    return 0;
}

static void remove_all_shapes_cb(cpShape *shape, void *data) {
    DArray *arr = data;
    DArray_push(arr, shape);
}

static void remove_all_bodies_cb(cpBody *body, void *data) {
    DArray *arr = data;
    DArray_push(arr, body);
}

void ChipmunkScene_stop(struct Scene *scene, Engine *engine) {
    if (!scene->started) return;

    Scripting_call_hook(engine->scripting, scene, "cleanup");

    ChipmunkSceneCtx *context = scene->context;

    BSTree_destroy(context->recorders);
    if (scene->space) {
        DArray *shapes = DArray_create(sizeof(cpShape *), 128);
        cpSpaceEachShape(scene->space, remove_all_shapes_cb, shapes);
        int i = 0;
        for (i = 0; i < DArray_count(shapes); i++) {
            cpShape *shape = DArray_get(shapes, i);
            cpSpaceRemoveShape(scene->space, shape);
        }
        DArray_destroy(shapes);

        DArray *bodies = DArray_create(sizeof(cpBody *), 128);
        cpSpaceEachBody(scene->space, remove_all_bodies_cb, bodies);
        for (i = 0; i < DArray_count(bodies); i++) {
            cpBody *body = DArray_get(bodies, i);
            cpSpaceRemoveBody(scene->space, body);
        }
        DArray_destroy(bodies);

        LIST_FOREACH(context->tile_shapes, first, next, current) {
            cpShape *shape = current->value;
            cpShapeFree(shape);
        }
        List_destroy(context->tile_shapes);
        free(context);

      cpSpaceFree(scene->space);
      scene->space = NULL;
    }

    scene->context = NULL;
    Stepper_reset(engine->physics->stepper);
    scene->started = 0;
}

void ChipmunkScene_start(struct Scene *scene, Engine *engine) {
    ChipmunkSceneCtx *context = calloc(1, sizeof(ChipmunkSceneCtx));

    context->scene = scene;
    context->engine = engine;

    scene->context = context;
    context->tile_shapes = List_create();
    context->recorders = BSTree_create((BSTree_compare)strcmp);
}

void ChipmunkScene_start_success_cb(struct Scene *scene, Engine *engine) {
    ChipmunkScene_create_space(scene, engine);
}

void ChipmunkScene_cleanup(struct Scene *scene, Engine *UNUSED(engine)) {
    check(scene != NULL, "No scene to destroy");
error:
    return;
}

int recorder_record_cb(BSTreeNode *node, void *UNUSED(context)) {
    Recorder *recorder = node->data;

    if (recorder->state == RecorderStateRecording) {
        size_t size;
        void *frame = recorder->_(capture_frame)(recorder, &size);
        Recorder_write_frame(recorder, frame, size);
    }

    return 0;
}

void ChipmunkScene_record_recorders(Scene *scene, Engine *UNUSED(engine)) {
    ChipmunkSceneCtx *context = scene->context;
    BSTree_traverse(context->recorders, recorder_record_cb, NULL);
}

int recorder_play_cb(BSTreeNode *node, void *context) {
    Recorder *recorder = node->data;
    Engine *engine = context;

    if (recorder->state == RecorderStatePlaying) {
        void *frame = Recorder_read_frame(recorder);
        if (frame) {
            recorder->_(apply_frame)(recorder, frame);
        } else {
            // We are done playing.
            Recorder_set_state(recorder, RecorderStateIdle);
            Scripting_call_hook(engine->scripting, recorder,
                    "finished_playing");
        }
    }

    return 0;
}

void ChipmunkScene_play_recorders(Scene *scene, Engine *engine) {
    ChipmunkSceneCtx *context = scene->context;
    BSTree_traverse(context->recorders, recorder_play_cb, engine);
}

void ChipmunkScene_add_recorder(Scene *scene, Engine *UNUSED(engine),
        struct Recorder *recorder) {
    assert(scene != NULL);
    ChipmunkSceneCtx *context = (ChipmunkSceneCtx *)scene->context;

    BSTree_set(context->recorders, recorder->id, recorder);
    ChipmunkRecorder_contextualize(recorder);
}

void ChipmunkScene_remove_recorder(Scene *scene, Engine *UNUSED(engine),
                           struct Recorder *recorder) {
    assert(scene != NULL);
    ChipmunkSceneCtx *context = (ChipmunkSceneCtx *)scene->context;
    BSTree_delete(context->recorders, recorder->id);
}

static inline int presolve_traverse_cb(BSTreeNode *node, void *context) {
    OCSTraverseCtx *ctx = (OCSTraverseCtx *)context;
    Entity *entity = node->data;
    Scripting_call_hook(ctx->engine->scripting, entity, "presolve");
    return 0;
}

void ChipmunkScene_update(struct Scene *scene, Engine *engine) {
    OCSTraverseCtx ctx = { .scene = scene, .engine = engine };
    BSTree_traverse(scene->entities, presolve_traverse_cb, &ctx);

    int phys_ticks = (int)(engine->frame_ticks < 100 ? engine->frame_ticks : 100);
    Stepper_update(engine->physics->stepper, phys_ticks);

    while (Stepper_pop(engine->physics->stepper)) {
        ChipmunkScene_play_recorders(scene, engine);
        cpSpaceStep(scene->space, engine->physics->stepper->step_skip / 1000.0);
        Scripting_call_hook(engine->scripting, scene, "postsolve");
        ChipmunkScene_record_recorders(scene, engine);
    }
}


static void render_shape_iter(cpShape *shape, void *data) {
    cpPolyShape *pshape = (cpPolyShape *)shape;
    OCSIterData *iter_data = (OCSIterData *)data;

    VPoint shape_verts[pshape->numVerts];

    int i = 0;
    cpVect *vert = pshape->verts;
    cpBody *body = cpShapeGetBody(shape);
    cpVect center;
    VPoint vcenter = {0, 0};
    float rot;
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
    Graphics_stroke_path(iter_data->engine->graphics, shape_verts,
                         pshape->numVerts, vcenter, color, 1, rot, 1, 0);
}

void ChipmunkScene_render_physdebug(struct Scene *scene, Engine *engine) {
    Graphics *graphics = ((Engine *)engine)->graphics;
    GfxShader *dshader = Graphics_get_shader(graphics, "decal");
    Graphics_project_camera(graphics, scene->camera);

    Graphics_use_shader(graphics, dshader);
    OCSIterData iter_data = {scene, engine};
    cpSpaceEachShape(scene->space, render_shape_iter, &iter_data);
}

void ChipmunkScene_render_debug_text(struct Scene *scene, Engine *engine) {
    Graphics_project_screen_camera(engine->graphics, scene->camera);
    Graphics_reset_modelview_matrix(engine->graphics);
    GLfloat white[4] = {1.0, 1.0, 1.0, 1.0};

    char *dTxt = malloc(256 * sizeof(char));
    sprintf(dTxt, "FPS CAP: %d", FPS);
    VPoint offset = {0,
                     -scene->camera->screen_size.h / 2.0 + engine->graphics->debug_font->face->height * 2 / 64};

    GfxFont *font = engine->graphics->debug_font;
    GLfloat black[4] = {0, 0, 0, 1};
    float shagnitude = MAX(font->px_size / 12, 1);
    VPoint shadow_offset = {shagnitude, shagnitude};
    Graphics_draw_string(engine->graphics, dTxt, engine->graphics->debug_font,
        white, offset, GfxTextAlignCenter, black, shadow_offset, 0);

    VPoint line = {0, engine->graphics->debug_font->px_size};
    offset = VPoint_add(offset, line);
    sprintf(dTxt, "ACTUAL: %.02f", 1000.0 / engine->frame_ticks);
    Graphics_draw_string(engine->graphics, dTxt, engine->graphics->debug_font,
        white, offset, GfxTextAlignCenter, black, shadow_offset, 0);

    free(dTxt);
}

void ChipmunkScene_render(struct Scene *scene, Engine *engine) {
    if (scene->started == 0) return;
    Graphics *graphics = ((Engine *)engine)->graphics;

    GfxShader *dshader = Graphics_get_shader(graphics, "decal");
    GfxShader *tshader = Graphics_get_shader(graphics, "tilemap");
    GfxShader *txshader = Graphics_get_shader(graphics, "text");

    Scene_fill(scene, engine, scene->bg_color, scene->bg_z, 0);

    if (scene->parallax) {
        Parallax_render(scene->parallax, engine->graphics);
    }

    Graphics_project_camera(graphics, scene->camera);
    if (scene->tile_map) {
        Graphics_use_shader(graphics, tshader);
        TileMap_render(scene->tile_map, graphics, scene->pixels_per_meter);
    }

    Graphics_project_camera(graphics, scene->camera);
    Scene_render_entities(scene, engine);
    Scene_render_selected_entities(scene, engine);

    if (scene->canvas) {
        Canvas_render(scene->canvas, engine);
    }

    Scene_render_overlays(scene, engine);
    Graphics_use_shader(graphics, dshader);
    Scene_fill(scene, engine, scene->cover_color, scene->cover_z, 0);

    if (scene->debug_camera) {
        Camera_debug(scene->camera, engine->graphics);
    }
    if (scene->draw_grid && scene->world) {
        Scene_draw_debug_grid(scene, graphics);
    }
    if (scene->render_mode == kSceneRenderModePhysicsDebug) {
        ChipmunkScene_render_physdebug(scene, engine);
    }
    if (scene->draw_debug_text) {
        Graphics_use_shader(graphics, txshader);
        ChipmunkScene_render_debug_text(scene, engine);
    }
}

void ChipmunkScene_control(struct Scene *scene, Engine *engine) {
    Input *input = engine->input;
    if (input->phys_render) {
        scene->render_mode =
            (scene->render_mode == kSceneRenderModeNormal ?
             kSceneRenderModePhysicsDebug : kSceneRenderModeNormal);
    }
    if (input->debug_scene_draw_grid) scene->draw_grid = !(scene->draw_grid);
}

VPoint ChipmunkScene_get_gravity(struct Scene *scene) {
    check(scene != NULL, "No scene to get gravity from");
    return scene->gravity;
error:
    return VPointZero;
}

void ChipmunkScene_set_gravity(struct Scene *scene, VPoint gravity) {
    check(scene != NULL, "No scene to set gravity on");
    scene->gravity = gravity;
    if (!scene->space) return;
    cpVect cp_grav = {gravity.x, gravity.y};
    cpSpaceSetGravity(scene->space, cp_grav);
error:
    return;
}

static int coll_build_arb_data(lua_State *L, void *context) {
  cpArbiter *arb = context;

  lua_newtable(L);

  // Total impulse
  cpVect imp = cpArbiterTotalImpulse(arb);
  lua_createtable(L, 2, 0);
  lua_pushnumber(L, 1);
  lua_pushnumber(L, imp.x);
  lua_settable(L, -3);
  lua_pushnumber(L, 2);
  lua_pushnumber(L, imp.y);
  lua_settable(L, -3);
  lua_setfield(L, -2, "total_impulse");

  return 1;
}

int collision_begin_cb(cpArbiter *arb, cpSpace *UNUSED(space),
        void *data) {
    cpBody *a_cpbody, *b_cpbody;
    cpArbiterGetBodies(arb, &a_cpbody, &b_cpbody);
    Body *a_body = cpBodyGetUserData(a_cpbody);
    Body *b_body = cpBodyGetUserData(b_cpbody);
    ChipmunkSceneCtx *ctx = data;

    Scripting_dhook_arg_closure closure = {
        .function = coll_build_arb_data,
        .context = arb
    };
    int is_first = cpArbiterIsFirstContact(arb);
    Scripting_call_dhook(ctx->engine->scripting, ctx->scene, "collision_begin",
                         LUA_TUSERDATA, a_body,
                         LUA_TUSERDATA, b_body,
                         LUA_TFUNCTION, &closure,
                         LUA_TBOOLEAN, is_first,
                         NULL);
    return 1;
}

int collision_pre_solve_cb(cpArbiter *arb, cpSpace *UNUSED(space),
        void *data) {
    cpBody *a_cpbody, *b_cpbody;
    cpArbiterGetBodies(arb, &a_cpbody, &b_cpbody);
    Body *a_body = cpBodyGetUserData(a_cpbody);
    Body *b_body = cpBodyGetUserData(b_cpbody);
    ChipmunkSceneCtx *ctx = data;

    Scripting_dhook_arg_closure closure = {
        .function = coll_build_arb_data,
        .context = arb
    };
    int is_first = cpArbiterIsFirstContact(arb);
    Scripting_call_dhook(ctx->engine->scripting, ctx->scene, "collision_pre_solve",
                         LUA_TUSERDATA, a_body,
                         LUA_TUSERDATA, b_body,
                         LUA_TFUNCTION, &closure,
                         LUA_TBOOLEAN, is_first,
                         NULL);
    return 1;
}

void collision_post_solve_cb(cpArbiter *arb, cpSpace *UNUSED(space),
        void *data) {
    cpBody *a_cpbody, *b_cpbody;
    cpArbiterGetBodies(arb, &a_cpbody, &b_cpbody);
    Body *a_body = cpBodyGetUserData(a_cpbody);
    Body *b_body = cpBodyGetUserData(b_cpbody);
    ChipmunkSceneCtx *ctx = data;

    Scripting_dhook_arg_closure closure = {
        .function = coll_build_arb_data,
        .context = arb
    };
    int is_first = cpArbiterIsFirstContact(arb);
    Scripting_call_dhook(ctx->engine->scripting, ctx->scene, "collision_post_solve",
                         LUA_TUSERDATA, a_body,
                         LUA_TUSERDATA, b_body,
                         LUA_TFUNCTION, &closure,
                         LUA_TBOOLEAN, is_first,
                         NULL);
}

void collision_separate_cb(cpArbiter *arb, cpSpace *UNUSED(space),
        void *data) {
    cpBody *a_cpbody, *b_cpbody;
    cpArbiterGetBodies(arb, &a_cpbody, &b_cpbody);
    Body *a_body = cpBodyGetUserData(a_cpbody);
    Body *b_body = cpBodyGetUserData(b_cpbody);
    ChipmunkSceneCtx *ctx = data;

    Scripting_dhook_arg_closure closure = {
        .function = coll_build_arb_data,
        .context = arb
    };
    int is_first = cpArbiterIsFirstContact(arb);
    Scripting_call_dhook(ctx->engine->scripting, ctx->scene, "collision_separate",
                         LUA_TUSERDATA, a_body,
                         LUA_TUSERDATA, b_body,
                         LUA_TFUNCTION, &closure,
                         LUA_TBOOLEAN, is_first,
                         NULL);
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

void ChipmunkScene_add_entity_body(Scene *scene, Engine *engine,
        Entity *entity) {
    assert(entity != NULL);
    assert(entity->body != NULL);
    Body *body = entity->body;

    body->cp_space = scene->space;
    body->state.engine = engine;
    body->state.entity = entity;
    body->state.scene = scene;

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

void ChipmunkScene_add_entity_cb(Scene *scene, Engine *engine,
        Entity *entity) {
    if (scene->space) {
        ChipmunkScene_add_entity_body(scene, engine, entity);
    }
}

void ChipmunkScene_remove_entity_cb(Scene *scene, Engine *UNUSED(engine),
        Entity *entity) {
    if (scene->space) {
      entity->body->_(cleanup)(entity->body);
    }
}

void hit_test_func(cpShape *shape, cpFloat UNUSED(distance),
        cpVect UNUSED(point), void *data) {
    if (shape->collision_type == OCSCollisionTypeEntity) {
        Entity **top = data;
        cpBody *cpb = shape->body;
        Body *body = cpb->data;
        if (*top == NULL || body->state.entity->z < (*top)->z) {
            *top = body->state.entity;
        }
    }
}

Entity *ChipmunkScene_hit_test(Scene *scene, VPoint g_point) {
    VPoint world_point = VPoint_scale(g_point, 1.0 / scene->pixels_per_meter);
    Entity *top = NULL;
    cpVect wp = {world_point.x, world_point.y};
    cpSpaceNearestPointQuery(scene->space, wp, 0.0, CP_ALL_LAYERS,
                             CP_NO_GROUP, hit_test_func, &top);
    return top;
}

void ChipmunkScene_create_collision_shapes(Scene *scene,
        Engine *UNUSED(engine)) {
    ChipmunkSceneCtx *context = scene->context;
    if (scene->tile_map) {
        TileMap *map = scene->tile_map;
        if (map->collision_shapes) {
            int i = 0;
            cpBody *map_body = cpSpaceGetStaticBody(scene->space);
            float grid_size = scene->tile_map->meters_per_tile;
            for (i = 0; i < DArray_count(map->collision_shapes); i++) {
                VPolygon *poly = DArray_get(map->collision_shapes, i);

                // Turn it into something chipmunk likes
                VPolygon_wind(poly, 0);
                cpVect verts[poly->num_points];
                int j = 0;
                for (j = 0; j < poly->num_points; j++) {
                    VPoint point = VPolygon_get_point(poly, j);
                    cpVect cp_point = {point.x * grid_size,
                                       point.y * grid_size};
                    verts[j] = cp_point;
                }
                cpVect cp_origin = {poly->origin.x * grid_size,
                                    poly->origin.y * grid_size};
                cpShape *tile_shape =
                    cpPolyShapeNew(map_body, poly->num_points, verts, cp_origin);
                cpShapeSetFriction(tile_shape, 0.5);
                tile_shape->collision_type = OCSCollisionTypeTile;
                cpSpaceAddStaticShape(scene->space, tile_shape);
                List_push(context->tile_shapes, tile_shape);
            }
        } else {
            // Create a shape for each tile

            if (DArray_count(scene->tile_map->layers) > 0) {
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
        }
    }
}

static inline int add_body_traverse_cb(BSTreeNode *node, void *context) {
    OCSTraverseCtx *ctx = (OCSTraverseCtx *)context;
    Entity *entity = node->data;
    ChipmunkScene_add_entity_body(ctx->scene, ctx->engine, entity);
    return 0;
}

int ChipmunkScene_create_space(Scene *scene, Engine *engine) {
    check_mem(scene);
    check_mem(engine);

    scene->space = cpSpaceNew();
    ChipmunkSceneCtx *context = scene->context;
    context->space = scene->space;

    cpVect gravity = {scene->gravity.x, scene->gravity.y};
    cpSpaceSetGravity(scene->space, gravity);
    cpSpaceSetCollisionSlop(scene->space, COLLISION_SLOP);
    cpSpaceSetCollisionBias(scene->space, 0.1);
    cpSpaceSetSleepTimeThreshold(scene->space, 0.4);
    cpSpaceSetIdleSpeedThreshold(scene->space, 1.0);

    cpSpaceAddCollisionHandler(scene->space, OCSCollisionTypeEntity,
                               OCSCollisionTypeEntity, collision_begin_cb,
                               collision_pre_solve_cb,
                               collision_post_solve_cb,
                               collision_separate_cb, context);
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

    ChipmunkScene_create_collision_shapes(scene, engine);

    OCSTraverseCtx ctx = { .scene = scene, .engine = engine };
    BSTree_traverse(scene->entities, add_body_traverse_cb, &ctx);

    return 1;
error:
    return 0;
}

SceneProto ChipmunkSceneProto = {
    .kind = kSceneKindChipmunk,
    .start = ChipmunkScene_start,
    .start_success_cb = ChipmunkScene_start_success_cb,
    .stop = ChipmunkScene_stop,
    .cleanup = ChipmunkScene_cleanup,
    .update = ChipmunkScene_update,
    .render = ChipmunkScene_render,
    .control = ChipmunkScene_control,
    .add_entity_cb = ChipmunkScene_add_entity_cb,
    .remove_entity_cb = ChipmunkScene_remove_entity_cb,
    .hit_test = ChipmunkScene_hit_test,
    .add_recorder = ChipmunkScene_add_recorder,
    .remove_recorder = ChipmunkScene_remove_recorder,
    .get_gravity = ChipmunkScene_get_gravity,
    .set_gravity = ChipmunkScene_set_gravity
};
