#include "easer_bindings.h"
#include "engine.h"

const char *luab_Easer_lib = "dab_easer";
const char *luab_Easer_metatable = "DaBes.easer";

Scripting_num_getter(Easer, value);
Scripting_num_setter(Easer, value);
Scripting_num_getter(Easer, time_scale);
Scripting_num_setter(Easer, time_scale);

int luab_Easer_new(lua_State *L) {
    Engine *engine = luaL_get_engine(L);
    Easer_userdata *ud = NULL;

    int length_ms = lua_tonumber(L, 1);
    ud = lua_newuserdata(L, sizeof(Easer_userdata));
    check(ud != NULL, "Could not make Easer userdata");
    ud->p = NULL;
  
    luaL_getmetatable(L, luab_Easer_metatable);
    lua_setmetatable(L, -2);

    Easer *easer = Engine_gen_easer(engine, length_ms, NULL);
    luaL_register_ud(L, -1, (void **)&ud->p, easer);
    return 1;
error:
    return 0;
}

int luab_Easer_close(lua_State *L) {
    Easer_userdata *ud = (Easer_userdata *)
        luaL_testudata(L, 1, luab_Easer_metatable);
    ud->p = NULL;
    return 0;
}

static const struct luaL_Reg luab_Easer_meths[] = {
    {"__gc", luab_Easer_close},
    {"get_value", luab_Easer_get_value},
    {"set_value", luab_Easer_set_value},
    {"get_time_scale", luab_Easer_get_time_scale},
    {"set_time_scale", luab_Easer_set_time_scale},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Easer_funcs[] = {
    {"new", luab_Easer_new},
    {NULL, NULL}
};

int luaopen_dabes_easer(lua_State *L) {
    luaL_newmetatable(L, luab_Easer_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Easer_meths, 0);
    luaL_newlib(L, luab_Easer_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Easer_lib);

    return 1;
}
