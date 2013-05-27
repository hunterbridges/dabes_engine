#include "scene_bindings.h"

#include <stdio.h>
#include "../core/engine.h"
#include "../audio/music_bindings.h"
#include "../graphics/parallax_bindings.h"
#include "scene.h"
#include "ortho_chipmunk_scene.h"

const char *luab_Scene_lib = "dab_scene";
const char *luab_Scene_metatable = "DaBes.scene";

int luab_Scene_new(lua_State *L) {
    Engine *engine = luaL_get_engine(L);

    Scene_userdata *scene_ud = lua_newuserdata(L, sizeof(Scene_userdata));
    check(scene_ud != NULL, "Could not make scene userdata");

    luaL_getmetatable(L, luab_Scene_metatable);
    lua_setmetatable(L, -2);

    Scene *scene = NULL;
    const char *proto = NULL;

    int valid_proto = 0;
    proto = lua_tostring(L, 1);
    if (streq(proto, "ortho_chipmunk")) {
        scene = Scene_create(engine, OrthoChipmunkSceneProto);
        valid_proto = 1;
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
        luaL_checkudata(L, 1, luab_Scene_metatable);
    if (scene_ud->p) Scene_destroy(scene_ud->p, engine);
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

    Scene_load_tile_map(scene, engine, (char *)file, 0);
    scene->tile_map->meters_per_tile = meters_per_tile;

    return 1;
error:
    return 0;
}

int luab_Scene_start(lua_State *L) {
    Engine *engine = luaL_get_engine(L);
    Scene *scene = luaL_toscene(L, 1);
    check(scene != NULL, "Scene required");
    if (!scene->started) {
        scene->_(start)(scene, engine);
    }

    return 1;
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
    check(scene != NULL, "Music required");

    lua_pop(L, 3);

    scene->music = music;
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
    check(parallax != NULL, "Parallax required");

    if (scene->tile_map) {
        GfxSize level_size = {
            scene->tile_map->cols * scene->tile_map->tile_size.w,
            scene->tile_map->rows * scene->tile_map->tile_size.h
        };
        parallax->level_size = level_size;
    }
    parallax->camera = scene->camera;

    lua_pop(L, 3);

    scene->parallax = parallax;
error:
    return 0;
}

// Property synthesis
Scripting_bool_getter(Scene, draw_grid);
Scripting_bool_setter(Scene, draw_grid);

Scripting_bool_getter(Scene, debug_camera);
Scripting_bool_setter(Scene, debug_camera);

static const struct luaL_Reg luab_Scene_meths[] = {
    {"__gc", luab_Scene_close},
    {"start", luab_Scene_start},
    {"load_map", luab_Scene_load_map},
    {"get_music", luab_Scene_get_music},
    {"set_music", luab_Scene_set_music},
    {"get_parallax", luab_Scene_get_parallax},
    {"set_parallax", luab_Scene_set_parallax},
    {"get_draw_grid", luab_Scene_get_draw_grid},
    {"set_draw_grid", luab_Scene_set_draw_grid},
    {"get_debug_camera", luab_Scene_get_debug_camera},
    {"set_debug_camera", luab_Scene_set_debug_camera},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Scene_funcs[] = {
    {"new", luab_Scene_new},
    {NULL, NULL}
};

int luaopen_dabes_scene(lua_State *L) {
    luaL_newmetatable(L, luab_Scene_metatable);

    /* metatable.__index = metatable */
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
    lua_getfield(L, -1, "get_current_scene");
    int result = lua_pcall(L, 0, 1, 0);
    if (result) Scripting_bail(L, "Failed to get current scene");

    Scene_userdata *scene_ud = (Scene_userdata *)lua_touserdata(L, -1);
    Scene *scene = scene_ud->p;
    lua_pop(L, 2);
    return scene;
}

////////////////////////////////////////////////////////////////////////////////

int Scene_configure(Scene *scene, Engine *engine) {
    check(scene != NULL, "No scene to configure");
    check(engine != NULL, "Need engine to configure scene");

    lua_State *L = engine->scripting->L;
    int result;

    lua_getglobal(L, scene->name);
    int n = lua_gettop(L);
    int start = n + 1;

    lua_getfield(L, -1, "configure");
    lua_pushnumber(L, scene->tile_map->cols * scene->tile_map->meters_per_tile);
    lua_pushnumber(L, scene->tile_map->rows * scene->tile_map->meters_per_tile);

    result = lua_pcall(L, 2, LUA_MULTRET, 0);
    if (result) Scripting_bail(L, "Failed to run script");
    n = lua_gettop(L) - n;

    int i = 1;
    for (i = start; i <= n; i++) {
        if (lua_istable(L, start)) {
            int mt = lua_getmetatable(L, start);
            if (!mt) {
                lua_pop(L, 1);
                continue;
            }
            lua_getfield(L, -1, "identifier");
            const char *identifier = lua_tostring(L, -1);
            lua_pop(L, 2);

            if (streq(identifier, SCRIPTING_CL_ENTITY_CONFIG)) {
                GameEntity *entity = GameEntity_create();

                lua_getfield(L, start, "sprite");
                int sprite = lua_gettop(L);

                lua_getfield(L, sprite, "cell_size");
                lua_getfield(L, -1, "w");
                lua_getfield(L, -2, "h");
                GfxSize cell_size = {
                    lua_tonumber(L, -2),
                    lua_tonumber(L, -1)
                };
                lua_pop(L, 3); // Pop w, h, cell_size

                lua_getfield(L, sprite, "texture");
                entity->sprite =
                    Graphics_sprite_from_image(engine->graphics,
                            (char *)lua_tostring(L, -1), cell_size);
                lua_pop(L, 1); // Pop texture

                lua_getfield(L, sprite, "num_animations");
                int num_animations = lua_tointeger(L, -1);
                lua_pop(L, 1);

                lua_getfield(L, sprite, "animations");
                int animations = lua_gettop(L);
                if (lua_istable(L, animations) && num_animations > 0) {
                    int a_idx = 0;
                    for (a_idx = 0; a_idx < num_animations; a_idx++) {
                        lua_pushnumber(L, a_idx + 1);
                        lua_gettable(L, animations);
                        int animation = lua_gettop(L);

                        lua_getfield(L, animation, "name");
                        const char *anim_name = lua_tostring(L, -1);

                        lua_getfield(L, animation, "num_frames");
                        lua_getfield(L, animation, "fps");
                        int num_frames = lua_tointeger(L, -2);
                        int fps = lua_tointeger(L, -1);
                        lua_pop(L, 2);

                        printf("%s -> ", anim_name);
                        printf("count(%d) ", num_frames);
                        printf("@ %d fps :: ", fps);

                        lua_getfield(L, animation, "frames");
                        int frames[num_frames];
                        int frames_mem = lua_gettop(L);
                        int f_idx = 0;
                        for (f_idx = 0; f_idx < num_frames; f_idx++) {
                            lua_pushnumber(L, f_idx + 1);
                            lua_gettable(L, frames_mem);
                            frames[f_idx] = lua_tointeger(L, -1);
                            lua_pop(L, 1); // Pop frame
                            printf("%d ", frames[f_idx]);
                        }

                        printf("\n");
                        Sprite_add_animation(entity->sprite, anim_name,
                                num_frames, frames, fps);
                        lua_pop(L, 3); // Pop frames, name, animation
                    }
                }
                lua_pop(L, 1); // Pop animations
                lua_pop(L, 1); // Pop sprite

                lua_getfield(L, start, "start_animation");
                lua_getfield(L, start, "current_frame");
                lua_getfield(L, start, "can_rotate");
                lua_getfield(L, start, "edge_friction");
                lua_getfield(L, start, "x");
                lua_getfield(L, start, "y");
                lua_getfield(L, start, "w");
                lua_getfield(L, start, "h");
                lua_getfield(L, start, "rotation");
                lua_getfield(L, start, "mass");
                lua_getfield(L, start, "alpha");
                Sprite_use_animation(entity->sprite, lua_tostring(L, -11));
                entity->current_frame = lua_tointeger(L, -10);
                entity->config.can_rotate = lua_toboolean(L, -9);
                entity->config.edge_friction = lua_tonumber(L, -8);
                entity->config.center.x = lua_tonumber(L, -7);
                entity->config.center.y = lua_tonumber(L, -6);
                entity->config.size.w = lua_tonumber(L, -5);
                entity->config.size.h = lua_tonumber(L, -4);
                entity->config.rotation = lua_tonumber(L, -3);
                entity->config.mass = lua_tonumber(L, -2);
                entity->alpha = lua_tonumber(L, -1);
                entity->pixels_per_meter = scene->pixels_per_meter;
                lua_pop(L, 11);

                List_push(scene->entities, entity);
            } else if (streq(identifier, SCRIPTING_CL_PARALLAX)) {
            }
        }
        lua_remove(L, start); // Remove result from bottom
    }

    lua_pop(L, 1); // Pop the scene global

    return 1;
error:
    return 0;
}
