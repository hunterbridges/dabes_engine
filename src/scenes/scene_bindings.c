#include "scene_bindings.h"

#include <stdio.h>
#include "../core/engine.h"
#include "../audio/music_bindings.h"
#include "../entities/entity_bindings.h"
#include "../graphics/parallax_bindings.h"
#include "scene.h"
#include "chipmunk_scene.h"
#include "static_scene.h"
#include "telemetry_splash_scene.h"
#include "../layers/canvas_bindings.h"
#include "../layers/overlay_bindings.h"
#include "../recorder/recorder_bindings.h"

const char *luab_Scene_lib = "dab_scene";
const char *luab_Scene_metatable = "DaBes.scene";

int luab_Scene_new(lua_State *L) {
    Engine *engine = luaL_get_engine(L);

    Scene_userdata *scene_ud = lua_newuserdata(L, sizeof(Scene_userdata));
    Scene *scene = NULL;
    check(scene_ud != NULL, "Could not make scene userdata");
    scene_ud->p = NULL;

    luaL_getmetatable(L, luab_Scene_metatable);
    lua_setmetatable(L, -2);

    const char *proto = NULL;

    int valid_proto = 1;
    proto = lua_tostring(L, 1);
    if (streq(proto, "chipmunk")) {
        scene = Scene_create(engine, ChipmunkSceneProto);
    } else if (streq(proto, "static")) {
        scene = Scene_create(engine, StaticSceneProto);
    } else if (streq(proto, "telemetry")) {
        scene = Scene_create(engine, TelemetrySplashSceneProto);
    } else {
        valid_proto = 0;
    }
    check(valid_proto, "Invalid scene type: %s", proto);

    float pixels_per_meter = lua_tonumber(L, 2);
    scene->pixels_per_meter = pixels_per_meter;

    luaL_register_ud(L, -1, (void **)&scene_ud->p, scene);

    return 1;
error:
    if (scene) Scene_destroy(scene, engine);
    return 0;
}

int luab_Scene_close(lua_State *L) {
    Engine *engine = luaL_get_engine(L);

    Scene_userdata *scene_ud = (Scene_userdata *)
        luaL_testudata(L, 1, luab_Scene_metatable);
    if (scene_ud->p) {
        Scene_destroy(scene_ud->p, engine);
    }
    scene_ud->p = NULL;
    return 0;
}

int luab_Scene_load_map(lua_State *L) {
    Engine *engine = luaL_get_engine(L);
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");

    check(lua_isstring(L, 2),
            "Please provide a file name of a .tmx map to load.");
    check(lua_isnumber(L, 3),
            "Please provide a meters-per-tile conversion factor");

    const char *file = lua_tostring(L, 2);
    float meters_per_tile = lua_tonumber(L, 3);

    Scene_load_tile_map(scene, engine, (char *)file, 0, meters_per_tile);

    return 1;
error:
    return 0;
}

int luab_Scene_start(lua_State *L) {
    Engine *engine = luaL_get_engine(L);
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    if (!scene->started) {
        Scene_start(scene, engine);
    }

    return 1;
error:
    return 0;
}

int luab_Scene_stop(lua_State *L) {
    Engine *engine = luaL_get_engine(L);
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    if (scene->started) {
        Scene_stop(scene, engine);
    }

    return 1;
error:
    return 0;
}

int luab_Scene_add_entity(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    Engine *engine = luaL_get_engine(L);
    check(scene != NULL, "Scene required");
    lua_getfield(L, 2, "real");
    Entity *entity = luaL_toentity(L, -1);
    Scene_add_entity(scene, engine, entity);

    return 0;
error:
    return 0;
}

int luab_Scene_remove_entity(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    Engine *engine = luaL_get_engine(L);
    check(scene != NULL, "Scene required");
    lua_getfield(L, 2, "real");
    Entity *entity = luaL_toentity(L, -1);
    Scene_remove_entity(scene, engine, entity);

    return 0;
error:
    return 0;
}

int luab_Scene_add_overlay(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    lua_getfield(L, 2, "real");
    Overlay *overlay = luaL_tooverlay(L, -1);
    Scene_add_overlay(scene, overlay);

    return 0;
error:
    return 0;
}

int luab_Scene_remove_overlay(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    lua_getfield(L, 2, "real");
    Overlay *overlay = luaL_tooverlay(L, -1);
    Scene_remove_overlay(scene, overlay);

    return 0;
error:
    return 0;
}

int luab_Scene_add_recorder(lua_State *L) {
    Engine *engine = luaL_get_engine(L);
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    lua_getfield(L, 2, "real");
    Recorder *recorder = luaL_torecorder(L, -1);
    check(recorder != NULL, "Recorder required");
    if (scene->proto.add_recorder) {
        scene->_(add_recorder)(scene, engine, recorder);
    }

    return 0;
error:
    return 0;
}

int luab_Scene_remove_recorder(lua_State *L) {
    Engine *engine = luaL_get_engine(L);
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    lua_getfield(L, 2, "real");
    Recorder *recorder = luaL_torecorder(L, -1);
    check(recorder != NULL, "Recorder required");
    if (scene->proto.remove_recorder) {
        scene->_(remove_recorder)(scene, engine, recorder);
    }

    return 0;
error:
    return 0;
}

int luab_Scene_get_music(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    if (scene->music == NULL) {
        lua_pushnil(L);
        return 1;
    }

    luaL_lookup_instance(L, scene->music);
    return 1;
error:
    return 0;
}

int luab_Scene_set_music(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");

    lua_getfield(L, -1, "real");
    Music *music = luaL_tomusic(L, -1);

    lua_pop(L, 3);

    Scene_set_music(scene, music);
error:
    return 0;
}

int luab_Scene_get_parallax(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    if (scene->parallax == NULL) {
        lua_pushnil(L);
        return 1;
    }

    luaL_lookup_instance(L, scene->parallax);
    return 1;
error:
    return 0;
}

int luab_Scene_set_parallax(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");

    lua_getfield(L, -1, "real");
    Parallax *parallax = luaL_toparallax(L, -1);

    if (parallax) {
        if (scene->tile_map) {
            GfxSize level_size = {
                scene->tile_map->cols * scene->tile_map->tile_size.w,
                scene->tile_map->rows * scene->tile_map->tile_size.h
            };
            parallax->level_size = level_size;
        }
        parallax->camera = scene->camera;

        lua_pop(L, 3);
    }

    scene->parallax = parallax;
error:
    return 0;
}

int luab_Scene_get_canvas(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    if (scene->canvas == NULL) {
        lua_pushnil(L);
        return 1;
    }

    luaL_lookup_instance(L, scene->canvas);
    return 1;
error:
    return 0;
}

int luab_Scene_set_canvas(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");

    lua_getfield(L, -1, "real");
    Canvas *canvas = luaL_tocanvas(L, -1);

    scene->canvas = canvas;
    canvas->scene = scene;
    lua_pop(L, 3);
error:
    return 0;
}

int luab_Scene_get_camera(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    if (scene->camera == NULL || !luaL_lookup_instance(L, scene->camera)) {
        lua_pushnil(L);
        return 1;
    }
    return 1;
error:
    return 0;
}

// Property synthesis
Scripting_bool_getter(Scene, draw_grid);
Scripting_bool_setter(Scene, draw_grid);

Scripting_bool_getter(Scene, debug_camera);
Scripting_bool_setter(Scene, debug_camera);

Scripting_VVector4_getter(Scene, bg_color);
Scripting_VVector4_setter(Scene, bg_color);

Scripting_VVector4_getter(Scene, cover_color);
Scripting_VVector4_setter(Scene, cover_color);

int luab_Scene_get_gravity(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    luaL_pushvpoint(L, scene->_(get_gravity)(scene));

    return 1;
error:
    return 0;
}

int luab_Scene_set_gravity(lua_State *L) {
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    VPoint gravity = luaL_tovpoint(L, 2);
    scene->_(set_gravity)(scene, gravity);

    return 0;
error:
    return 0;
}

Scripting_num_getter(Scene, started_at);

static const struct luaL_Reg luab_Scene_meths[] = {
    {"__gc", luab_Scene_close},
    {"start", luab_Scene_start},
    {"stop", luab_Scene_stop},
    {"add_entity", luab_Scene_add_entity},
    {"remove_entity", luab_Scene_remove_entity},
    {"add_overlay", luab_Scene_add_overlay},
    {"remove_overlay", luab_Scene_remove_overlay},
    {"add_recorder", luab_Scene_add_recorder},
    {"remove_recorder", luab_Scene_remove_recorder},
    {"load_map", luab_Scene_load_map},
    {"get_music", luab_Scene_get_music},
    {"set_music", luab_Scene_set_music},
    {"get_canvas", luab_Scene_get_canvas},
    {"set_canvas", luab_Scene_set_canvas},
    {"get_parallax", luab_Scene_get_parallax},
    {"set_parallax", luab_Scene_set_parallax},
    {"get_camera", luab_Scene_get_camera},
    {"get_draw_grid", luab_Scene_get_draw_grid},
    {"set_draw_grid", luab_Scene_set_draw_grid},
    {"get_debug_camera", luab_Scene_get_debug_camera},
    {"set_debug_camera", luab_Scene_set_debug_camera},
    {"get_bg_color", luab_Scene_get_bg_color},
    {"set_bg_color", luab_Scene_set_bg_color},
    {"get_cover_color", luab_Scene_get_cover_color},
    {"set_cover_color", luab_Scene_set_cover_color},
    {"get_gravity", luab_Scene_get_gravity},
    {"set_gravity", luab_Scene_set_gravity},
    {"get_started_at", luab_Scene_get_started_at},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Scene_funcs[] = {
    {"new", luab_Scene_new},
    {NULL, NULL}
};

int luaopen_dabes_scene(lua_State *L) {
    luaL_newmetatable(L, luab_Scene_metatable);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, luab_Scene_meths, 0);
    luaL_newlib(L, luab_Scene_funcs);

    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Scene_lib);

    return 1;
}

Scene *luaL_get_current_scene(lua_State *L) {
    lua_getglobal(L, "scene_manager");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return NULL;
    }

    lua_getfield(L, -1, "current_scene");
    if (lua_isnil(L, -1)) {
        lua_pop(L, 2);
        return NULL;
    }

    lua_getfield(L, -1, "real");
    Scene_userdata *scene_ud = (Scene_userdata *)lua_touserdata(L, -1);
    Scene *scene = NULL;
    if (scene_ud) {
        scene = scene_ud->p;
    }
    lua_pop(L, 3);
    return scene;
}

int luaL_flip_scene(lua_State *L) {
    lua_getglobal(L, "scene_manager");
    lua_getfield(L, -1, "flip_scene");
    lua_pushvalue(L, -2);
    int result = lua_pcall(L, 1, 0, 0);
    if (result != 0) {
        debug("Error in flip scene\n    %s", lua_tostring(L, -1));
        lua_pop(L, 1);
        return 0;
    }
    lua_pop(L, 1);
    return 1;
}
