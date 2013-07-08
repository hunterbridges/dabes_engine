#include "engine_bindings.h"

const char *luab_Engine_lib = "dab_engine";

int luab_Engine_ticks(lua_State *L) {
    Engine *engine = luaL_get_engine(L);
    lua_pushnumber(L, Engine_get_ticks(engine));
    return 1;
}

int luab_Engine_frame_ticks(lua_State *L) {
    Engine *engine = luaL_get_engine(L);
    lua_pushnumber(L, engine->frame_ticks);
    return 1;
}

static const struct luaL_Reg luab_Engine_funcs[] = {
    {"ticks", luab_Engine_ticks},
    {"frame_ticks", luab_Engine_frame_ticks},
    {NULL, NULL}
};

int luaopen_dabes_engine(lua_State *L) {
    luaL_newlib(L, luab_Engine_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Engine_lib);

    return 1;
}
