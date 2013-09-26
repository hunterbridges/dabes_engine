#include "net_match_bindings.h"
#include "../core/engine.h"

const char *luab_NetMatch_lib = "dab_net_match";
const char *luab_NetMatch_metatable = "DaBes.net_match";

int luab_NetMatch_new(lua_State *L) {
    NetMatch_userdata *ud = NULL;
    Engine *engine = luaL_get_engine(L);

    ud = lua_newuserdata(L, sizeof(NetMatch_userdata));
    check(ud != NULL, "Could not make NetMatch userdata");
    ud->p = NULL;
  
    luaL_getmetatable(L, luab_NetMatch_metatable);
    lua_setmetatable(L, -2);

    NetMatch *match = NetMatch_create(engine);
    luaL_register_ud(L, -1, (void **)&ud->p, match);
    return 1;
error:
    return 0;
}

Scripting_destroy_closer(NetMatch);

static const struct luaL_Reg luab_NetMatch_meths[] = {
    {"__gc", luab_NetMatch_close},
    {NULL, NULL}
};

static const struct luaL_Reg luab_NetMatch_funcs[] = {
    {"new", luab_NetMatch_new},
    {NULL, NULL}
};

int luaopen_dabes_netmatch(lua_State *L) {
    luaL_newmetatable(L, luab_NetMatch_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_NetMatch_meths, 0);
    luaL_newlib(L, luab_NetMatch_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_NetMatch_lib);

    return 1;
}
