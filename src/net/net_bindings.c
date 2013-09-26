#include "net_bindings.h"
#include "net_match_bindings.h"
#include "net_match.h"
#include "../core/engine.h"

int Net_call_found_match_hook(Net *net, struct Engine *engine, void *assoc) {
    lua_State *L = engine->scripting->L;
    Scripting *scripting = engine->scripting;
    
    int stack = 0;
    
    int result = luaL_lookup_instance(L, net);
    if (!result) return 0;
    stack++;
    
    lua_getfield(L, -1, "found_match");
    stack++;
    
    // Swap the places of Net instance and found_match
    lua_pushvalue(L, -2);
    lua_remove(L, -3);
    
    // Instantiate the NetMatch in Lua
    result = luaL_dostring(L, "require 'dabes.net_match'"); // lol
    assert(result == 0);
    check(result == 0, "Unable to require dabes.net_match");
    
    lua_getglobal(L, "NetMatch");
    stack++;
    
    lua_getfield(L, -1, "new");
    lua_pushvalue(L, -2); // NetMatch needs to be the param to NetMatch.new()
    
    // Let's remove the first NetMatch class.
    lua_remove(L, -3);
    stack--;
    
    result = lua_pcall(L, 1, 1, 1);
    if (result != 0) {
        const char *error = lua_tostring(L, -1);
        if (scripting->error_callback) {
            scripting->error_callback(error);
        } else {
            debug("Error instantiating NetMatch programatically: %s", error);
        }
        lua_pop(L, 1 + stack);
        return 0;
    }
    
    // Ok, now should have the NetMatch instance on the top of the stack.
    // Now stack is just <found_match, Net instance, NetMatch instance>
    
    // Associate the native representation with the NetMatch
    // We need the "real" netmatch
    lua_getfield(L, -1, "real");
    NetMatch *match = luaL_tonetmatch(L, -1);
    match->_(associate_native)(match, assoc);
    lua_pop(L, 1);
    
    // At last, call the hook
    result = lua_pcall(L, 2, 0, 1);
    stack -= 3;
    if (result != 0) {
        const char *error = lua_tostring(L, -1);
        if (scripting->error_callback) {
            scripting->error_callback(error);
        } else {
            debug("Error calling found_match hook: %s", error);
        }
        lua_pop(L, 1);
        return 0;
    }
    
    return 1;
error:
    lua_pop(L, stack);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
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
