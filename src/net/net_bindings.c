#include "net_bindings.h"
#include "net_match_bindings.h"
#include "net_match.h"
#include "../core/engine.h"

const char *luab_Net_lib = "dab_net";
const char *luab_Net_metatable = "DaBes.net";

int luab_Net_new(lua_State *L) {
    Net_userdata *ud = NULL;
    Engine *engine = luaL_get_engine(L);

    ud = lua_newuserdata(L, sizeof(Net_userdata));
    check(ud != NULL, "Could not make Net userdata");
    ud->p = NULL;

    luaL_getmetatable(L, luab_Net_metatable);
    lua_setmetatable(L, -2);

    Net *net = engine->net;
    luaL_register_ud(L, -1, (void **)&ud->p, net);
    return 1;
error:
    return 0;
}

Scripting_null_closer(Net);

int luab_Net_authenticate(lua_State *L) {
    Net *net = luaL_tonet(L, 1);
    check(net != NULL, "Net required");

    if (net->proto.authenticate) {
        Engine *engine = luaL_get_engine(L);
        net->_(authenticate)(net, engine);
    } else {
        luaL_error(L, "Method not supported on this platform.");
    }
    return 0;
error:
    return 0;
}

int luab_Net_find_matches(lua_State *L) {
    Net *net = luaL_tonet(L, 1);
    check(net != NULL, "Net required");

    if (net->proto.find_matches) {
        Engine *engine = luaL_get_engine(L);
        net->_(find_matches)(net, engine);
    } else {
        luaL_error(L, "Method not supported on this platform.");
    }
    return 0;
error:
    return 0;
}

static const struct luaL_Reg luab_Net_meths[] = {
    {"__gc", luab_Net_close},
    {"authenticate", luab_Net_authenticate},
    {"find_matches", luab_Net_find_matches},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Net_funcs[] = {
    {"new", luab_Net_new},
    {NULL, NULL}
};

int luaopen_dabes_net(lua_State *L) {
    luaL_newmetatable(L, luab_Net_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Net_meths, 0);
    luaL_newlib(L, luab_Net_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Net_lib);

    return 1;
}
