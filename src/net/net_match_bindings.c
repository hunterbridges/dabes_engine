#include "net_match_bindings.h"
#include "../core/engine.h"
#include "../recorder/recorder_bindings.h"

const char *luab_NetMatch_lib = "dab_net_match";
const char *luab_NetMatch_metatable = "DaBes.net_match";

NetMatch *luaL_instantiate_netmatch(lua_State *L) {
    int irc = luaL_dostring(L, "require 'dabes.net_match'");
    check(irc == 0, "Unable to require 'dabes.net_match'");

    lua_getglobal(L, "NetMatch");
    lua_getfield(L, -1, "new");
    lua_pushvalue(L, -2);
    lua_remove(L, -3);

    irc = lua_pcall(L, 1, 1, 1);
    if (irc != 0) {
        const char *error = lua_tostring(L, -1);
        debug("Error instantiating NetMatch via injection: %s", error);
        lua_pop(L, 1);
        return NULL;
    }

    lua_getfield(L, -1, "real");
    NetMatch *match = luaL_tonetmatch(L, -1);
    lua_pop(L, 1);

    return match;
error:
    return NULL;
}

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

int luab_NetMatch_handshake(lua_State *L) {
    NetMatch *match = luaL_tonetmatch(L, 1);
    check(match != NULL, "NetMatch required");

    if (match->proto.handshake) {
        Engine *engine = luaL_get_engine(L);
        match->_(handshake)(match, engine);
    } else {
        luaL_error(L, "Method not supported on this platform.");
    }
    return 0;
error:
    return 0;
}

int luab_NetMatch_get_metadata(lua_State *L) {
    NetMatch *match = luaL_tonetmatch(L, 1);
    check(match != NULL, "NetMatch required");

    if (match->proto.get_metadata) {
        Engine *engine = luaL_get_engine(L);
        match->_(get_metadata)(match, engine);
    } else {
        luaL_error(L, "Method not supported on this platform.");
    }
    return 0;
error:
    return 0;
}

int luab_NetMatch_get_player_count(lua_State *L) {
    NetMatch *match = luaL_tonetmatch(L, 1);
    check(match != NULL, "NetMatch required");

    if (match->proto.get_player_count) {
        Engine *engine = luaL_get_engine(L);
        int count = match->_(get_player_count)(match, engine);
        lua_pushinteger(L, count);
    } else {
        luaL_error(L, "Method not supported on this platform.");
    }
    return 1;
error:
    return 0;
}

int luab_NetMatch_get_player_number(lua_State *L) {
    NetMatch *match = luaL_tonetmatch(L, 1);
    check(match != NULL, "NetMatch required");

    if (match->proto.get_player_number) {
        Engine *engine = luaL_get_engine(L);
        int n = match->_(get_player_number)(match, engine);
        lua_pushinteger(L, n);
    } else {
        luaL_error(L, "Method not supported on this platform.");
    }
    return 1;
error:
    return 0;
}

int luab_NetMatch_send_null_msg(lua_State *L) {
    NetMatch *match = luaL_tonetmatch(L, 1);
    check(match != NULL, "NetMatch required");

    if (match->proto.send_msg) {
        lua_Integer to = lua_tointeger(L, 2);
        NetMatchMsg *msg = NetMatchMsg_null(0, to);

        Engine *engine = luaL_get_engine(L);
        match->_(send_msg)(match, engine, msg);

        NetMatchMsg_destroy(msg);
    } else {
        luaL_error(L, "Method not supported on this platform.");
    }
    return 0;
error:
    return 0;
}

int luab_NetMatch_send_packed_recorder_msg(lua_State *L) {
    NetMatch *match = luaL_tonetmatch(L, 1);
    check(match != NULL, "NetMatch required");

    if (match->proto.send_msg) {
        lua_Integer to = lua_tointeger(L, 2);
        Recorder *recorder = luaL_torecorder(L, 3);

        NetMatchMsg *msg = NetMatchMsg_packed_recorder(0, to, recorder);
        Engine *engine = luaL_get_engine(L);

        match->_(send_msg)(match, engine, msg);

        NetMatchMsg_destroy(msg);
    } else {
        luaL_error(L, "Method not supported on this platform.");
    }
    return 0;
error:
    return 0;
}

static const struct luaL_Reg luab_NetMatch_meths[] = {
    {"__gc", luab_NetMatch_close},
    {"handshake", luab_NetMatch_handshake},
    {"get_metadata", luab_NetMatch_get_metadata},
    {"get_player_count", luab_NetMatch_get_player_count},
    {"get_player_number", luab_NetMatch_get_player_number},

    {"send_null_msg", luab_NetMatch_send_null_msg},
    {"send_packed_recorder_msg", luab_NetMatch_send_packed_recorder_msg},

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
