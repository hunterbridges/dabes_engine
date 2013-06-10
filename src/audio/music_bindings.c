#include "music_bindings.h"

#include "../core/engine.h"

const char *luab_Music_lib = "dab_music";
const char *luab_Music_metatable = "DaBes.music";

int luab_Music_new(lua_State *L) {
    Music_userdata *music_ud = NULL;
    Engine *engine = luaL_get_engine(L);

    int c = lua_gettop(L);
    const char *files[c];

    music_ud = lua_newuserdata(L, sizeof(Music_userdata));
    check(music_ud != NULL, "Could not make music userdata");

    luaL_getmetatable(L, luab_Music_metatable);
    lua_setmetatable(L, -2);

    int i = 0;
    for (i = 0; i < c; i++) {
        files[i] = lua_tostring(L, 1);
        lua_remove(L, 1);
    }

    Music *music = Audio_gen_music(engine->audio, c, files);

    luaL_register_ud(L, -1, (void **)&music_ud->p, music);
    return 1;
error:
    return 0;
}

int luab_Music_close(lua_State *L) {
    Music_userdata *music_ud = (Music_userdata *)
        luaL_checkudata(L, 1, luab_Music_metatable);
    if (music_ud->p) {
        // The Audio engine will clean this up
        Music_end(music_ud->p);
    }
    music_ud->p = NULL;
    return 0;
}

int luab_Music_play(lua_State *L) {
    Music *music = luaL_tomusic(L, 1);
    check(music != NULL, "Music required");
    Music_play(music);

    return 1;
error:
    return 0;
}

int luab_Music_pause(lua_State *L) {
    Music *music = luaL_tomusic(L, 1);
    check(music != NULL, "Music required");
    Music_pause(music);

    return 1;
error:
    return 0;
}

int luab_Music_stop(lua_State *L) {
    Music *music = luaL_tomusic(L, 1);
    check(music != NULL, "Music required");
    Music_end(music);

    return 1;
error:
    return 0;
}

Scripting_num_getter(Music, volume);

int luab_Music_set_volume(lua_State *L) {
    Music *music = luaL_tomusic(L, 1);
    check(music != NULL, "Music required");
    float volume = lua_tonumber(L, 2);
    Music_set_volume(music, volume);

    return 1;
error:
    return 0;
}

static const struct luaL_Reg luab_Music_meths[] = {
    {"__gc", luab_Music_close},
    {"play", luab_Music_play},
    {"pause", luab_Music_pause},
    {"stop", luab_Music_stop},
    {"get_volume", luab_Music_get_volume},
    {"set_volume", luab_Music_set_volume},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Music_funcs[] = {
    {"new", luab_Music_new},
    {NULL, NULL}
};

int luaopen_dabes_music(lua_State *L) {
    luaL_newmetatable(L, luab_Music_metatable);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, luab_Music_meths, 0);
    luaL_newlib(L, luab_Music_funcs);

    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Music_lib);

    return 1;
}
