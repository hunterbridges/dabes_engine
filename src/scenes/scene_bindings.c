#include "scene_bindings.h"

#include <stdio.h>
#include "../core/engine.h"
#include "../core/scripting.h"
#include "scene.h"
#include "ortho_chipmunk_scene.h"

static const char *luab_Scene_metatable = "DaBes.scene";
static const char *luab_Scene_lib = "SceneBinding";

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

    scene_ud->scene = scene;

    return 1;
error:
    if (scene) Scene_destroy(scene, engine);
    return 0;
}

int luab_Scene_close(lua_State *L) {
    Engine *engine = luaL_get_engine(L);

    Scene_userdata *scene_ud = (Scene_userdata *)
        luaL_checkudata(L, 1, luab_Scene_metatable);
    if (scene_ud->scene) Scene_destroy(scene_ud->scene, engine);
    scene_ud->scene = NULL;
    return 0;
}

int luab_Scene_load_map(lua_State *L) {
    Engine *engine = luaL_get_engine(L);
    Scene_userdata *scene_ud = (Scene_userdata *)
        luaL_checkudata(L, 1, luab_Scene_metatable);
    check(lua_isstring(L, 2),
            "Please provide a file name of a .tmx map to load.");
    check(lua_isnumber(L, 3),
            "Please provide a meters-per-tile conversion factor");

    const char *file = lua_tostring(L, 2);
    float meters_per_tile = lua_tonumber(L, 3);

    Scene *scene = scene_ud->scene;
    Scene_load_tile_map(scene, engine, (char *)file, 0);
    scene->tile_map->meters_per_tile = meters_per_tile;

    return 1;
error:
    return 0;
}

// Property synthesis
Scripting_bool_getter(luab_Scene_get_draw_grid, luab_Scene_metatable,
    Scene_userdata, scene, Scene, draw_grid);
Scripting_bool_setter(luab_Scene_set_draw_grid, luab_Scene_metatable,
    Scene_userdata, scene, Scene, draw_grid);

Scripting_bool_getter(luab_Scene_get_debug_camera, luab_Scene_metatable,
    Scene_userdata, scene, Scene, debug_camera);
Scripting_bool_setter(luab_Scene_set_debug_camera, luab_Scene_metatable,
    Scene_userdata, scene, Scene, debug_camera);

static const struct luaL_Reg luab_Scene_meths[] = {
    {"__gc", luab_Scene_close},
    {"load_map", luab_Scene_load_map},
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
    lua_setglobal(L, "dab_scene");

    return 1;
}

Scene *luaL_get_current_scene(lua_State *L) {
    lua_getglobal(L, "scene_manager");
    lua_getfield(L, -1, "get_current_scene");
    int result = lua_pcall(L, 0, 1, 0);
    if (result) Scripting_bail(L, "Failed to get current scene");

    Scene_userdata *scene_ud = (Scene_userdata *)lua_touserdata(L, -1);
    Scene *scene = scene_ud->scene;
    lua_pop(L, 2);
    return scene;
}

////////////////////////////////////////////////////////////////////////////////

int Scene_init(Scene *scene, Engine *engine) {
    check(scene != NULL, "No scene to configure");
    check(engine != NULL, "Need engine to configure scene");

    lua_State *L = engine->scripting->L;
    lua_getglobal(L, scene->name);

    lua_getfield(L, -1, "pixels_per_meter");
    if (lua_type(L, -1) == LUA_TNUMBER) {
        scene->pixels_per_meter = lua_tointeger(L, -1);
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "music");
    if (lua_type(L, -1) == LUA_TTABLE) {
        const char *intro = NULL;
        const char *loop = NULL;

        int music_field = lua_gettop(L);
        lua_getfield(L, music_field, "intro");
        lua_getfield(L, music_field, "loop");
        if (lua_type(L, -2) == LUA_TSTRING) intro = lua_tostring(L, -2);
        if (lua_type(L, -1) == LUA_TSTRING) loop = lua_tostring(L, -1);
        lua_pop(L, 2);

        const char *files[2] = {intro, loop};
        scene->music = Audio_gen_music(engine->audio, 2, files);
    } else if (lua_type(L, -1) == LUA_TSTRING) {
        const char *files[1] = {(char *)lua_tostring(L, -1)};
        scene->music = Audio_gen_music(engine->audio, 1, files);
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "map");
    if (lua_type(L, -1) == LUA_TSTRING) {
        const char *map = lua_tostring(L, -1);
        Scene_load_tile_map(scene, engine, (char *)map, 0);
    } else if (lua_type(L, -1) == LUA_TTABLE) {
        int map = lua_gettop(L);
        lua_getfield(L, map, "file");
        lua_getfield(L, map, "meters_per_tile");
        const char *file = lua_tostring(L, -2);
        float meters_per_tile = lua_tonumber(L, -1);

        Scene_load_tile_map(scene, engine, (char *)file, 0);
        scene->tile_map->meters_per_tile = meters_per_tile;
        lua_pop(L, 2);
    }
    lua_pop(L, 1);

    lua_pop(L, 1); // Pop the scene global
    return 1;
error:
    return 0;
}

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
                GfxSize level_size = {
                    scene->tile_map->cols * scene->tile_map->tile_size.w,
                    scene->tile_map->rows * scene->tile_map->tile_size.h
                };
                scene->parallax = Parallax_create(level_size, scene->camera);
                Parallax *p = scene->parallax;

                lua_getfield(L, start, "y_wiggle");
                lua_getfield(L, start, "sea_level");
                p->y_wiggle = lua_tonumber(L, -2);
                p->sea_level = lua_tonumber(L, -1);
                lua_pop(L, 2);

                lua_getfield(L, start, "sky_color");
                int sky_color = lua_gettop(L);
                lua_getfield(L, sky_color, "r");
                lua_getfield(L, sky_color, "g");
                lua_getfield(L, sky_color, "b");
                lua_getfield(L, sky_color, "a");
                p->sky_color.rgba.r = lua_tonumber(L, -4);
                p->sky_color.rgba.g = lua_tonumber(L, -3);
                p->sky_color.rgba.b = lua_tonumber(L, -2);
                p->sky_color.rgba.a = lua_tonumber(L, -1);
                lua_pop(L, 5);

                lua_getfield(L, start, "sea_color");
                int sea_color = lua_gettop(L);
                lua_getfield(L, sea_color, "r");
                lua_getfield(L, sea_color, "g");
                lua_getfield(L, sea_color, "b");
                lua_getfield(L, sea_color, "a");
                p->sea_color.rgba.r = lua_tonumber(L, -4);
                p->sea_color.rgba.g = lua_tonumber(L, -3);
                p->sea_color.rgba.b = lua_tonumber(L, -2);
                p->sea_color.rgba.a = lua_tonumber(L, -1);
                lua_pop(L, 5);

                lua_getfield(L, start, "num_layers");
                int num_layers = lua_tointeger(L, -1);
                lua_pop(L, 1);

                lua_getfield(L, start, "layers");
                int layers = lua_gettop(L);
                int l_idx = 1;
                for (l_idx = 1; l_idx <= num_layers; l_idx++) {
                    lua_pushnumber(L, l_idx);
                    lua_gettable(L, layers);
                    int curlayer = lua_gettop(L);

                    lua_getfield(L, curlayer, "p_factor");
                    lua_getfield(L, curlayer, "texture");
                    lua_getfield(L, curlayer, "offset_x");
                    lua_getfield(L, curlayer, "offset_y");
                    lua_getfield(L, curlayer, "base_scale");
                    lua_getfield(L, curlayer, "y_wiggle");
                    float p_factor = lua_tonumber(L, -6);
                    const char *texname = lua_tostring(L, -5);
                    VPoint offset = {
                        .x = lua_tonumber(L, -4),
                        .y = lua_tonumber(L, -3)
                    };
                    float base_scale = lua_tonumber(L, -2);
                    float y_wiggle = lua_tonumber(L, -1);

                    GfxTexture *texture =
                        Graphics_texture_from_image(engine->graphics,
                            (char *)texname);
                    Parallax_add_layer(p, texture, p_factor, offset, base_scale,
                            y_wiggle);
                    lua_pop(L, 6);
                    lua_pop(L, 1); // Pop the layer
                }
                lua_pop(L, 1); // Pop the layers array
            }
        }
        lua_remove(L, start); // Remove result from bottom
    }

    lua_pop(L, 1); // Pop the scene global

    return 1;
error:
    return 0;
}
