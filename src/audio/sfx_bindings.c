#include "sfx_bindings.h"

#include "../core/engine.h"

const char *luab_Sfx_lib = "dab_sfx";
const char *luab_Sfx_metatable = "DaBes.sfx";

int luab_Sfx_new(lua_State *L) {
    Sfx_userdata *sfx_ud = NULL;

    Engine *engine = luaL_get_engine(L);

    const char *file = lua_tostring(L, 1);

    sfx_ud = lua_newuserdata(L, sizeof(Sfx_userdata));
    check(sfx_ud != NULL, "Could not make sfx userdata");

    luaL_getmetatable(L, luab_Sfx_metatable);
    lua_setmetatable(L, -2);

    Sfx *sfx = Audio_gen_sfx(engine->audio, file);

    luaL_register_ud(L, -1, (void **)&sfx_ud->p, sfx);
    return 1;
error:
    return 0;
}

Scripting_null_closer(Sfx);

int luab_Sfx_play(lua_State *L) {
    Sfx *sfx = luaL_tosfx(L, 1);
    check(sfx != NULL, "Sfx required");
    Sfx_play(sfx);

    return 1;
error:
    return 0;
}

Scripting_num_getter(Sfx, volume);

int luab_Sfx_set_volume(lua_State *L) {
    Sfx *sfx = luaL_tosfx(L, 1);
    check(sfx != NULL, "Sfx required");
    float volume = lua_tonumber(L, 2);
    Sfx_set_volume(sfx, volume);

    return 1;
error:
    return 0;
}

static const struct luaL_Reg luab_Sfx_meths[] = {
    {"__gc", luab_Sfx_close},
    {"play", luab_Sfx_play},
    {"get_volume", luab_Sfx_get_volume},
    {"set_volume", luab_Sfx_set_volume},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Sfx_funcs[] = {
    {"new", luab_Sfx_new},
    {NULL, NULL}
};

int luaopen_dabes_sfx(lua_State *L) {
    luaL_newmetatable(L, luab_Sfx_metatable);

    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");

    luaL_setfuncs(L, luab_Sfx_meths, 0);
    luaL_newlib(L, luab_Sfx_funcs);

    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Sfx_lib);

    return 1;
}
