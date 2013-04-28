#include <stdio.h>
#include "scene.h"

int Scene_init(Scene *scene, Engine *engine) {
    check(scene != NULL, "No scene to configure");
    check(engine != NULL, "Need engine to configure scene");

    lua_State *L = engine->scripting->L;
    lua_getglobal(L, scene->name);

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

        if (intro && loop) {
            scene->music = Music_load((char *)intro, (char *)loop);
        } else if (intro) {
            scene->music = Music_load((char *)intro, NULL);
        } else if (loop) {
            scene->music = Music_load((char *)loop, NULL);
        }
    } else if (lua_type(L, -1) == LUA_TSTRING) {
        scene->music = Music_load((char *)lua_tostring(L, -1), NULL);
    }

    if (scene->music) {
        Music_play(scene->music);
    }
    lua_pop(L, 1);

    lua_getfield(L, -1, "map");
    if (lua_type(L, -1) == LUA_TSTRING) {
        const char *map = lua_tostring(L, -1);
        Scene_load_tile_map(scene, engine, (char *)map, 0);
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
    lua_pushnumber(L, scene->tile_map->cols * PHYS_DEFAULT_GRID_SIZE);
    lua_pushnumber(L, scene->tile_map->rows * PHYS_DEFAULT_GRID_SIZE);

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
                lua_pop(L, 2); // Pop texture, sprite

                lua_getfield(L, start, "current_frame");
                lua_getfield(L, start, "x");
                lua_getfield(L, start, "y");
                lua_getfield(L, start, "w");
                lua_getfield(L, start, "h");
                lua_getfield(L, start, "rotation");
                lua_getfield(L, start, "mass");
                lua_getfield(L, start, "alpha");
                entity->current_frame = lua_tointeger(L, -8);
                entity->config.center.x = lua_tonumber(L, -7);
                entity->config.center.y = lua_tonumber(L, -6);
                entity->config.size.w = lua_tonumber(L, -5);
                entity->config.size.h = lua_tonumber(L, -4);
                entity->config.rotation = lua_tonumber(L, -3);
                entity->config.mass = lua_tonumber(L, -2);
                entity->alpha = lua_tonumber(L, -1);
                lua_pop(L, 7);

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

                    GfxTexture *texture = Graphics_texture_from_image(engine->graphics,
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
