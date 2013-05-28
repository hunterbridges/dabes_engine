#include "controller_bindings.h"

const char *luab_Controller_lib = "dab_controller";
const char *luab_Controller_metatable = "DaBes.controller";

static const struct luaL_Reg luab_Controller_meths[] = {
    {"__gc", luab_Controller_close},
    {"is_up", luab_Controller_is_up},
    {"is_down", luab_Controller_is_down},
    {"is_left", luab_Controller_is_left},
    {"is_right", luab_Controller_is_right},
    {"is_jumping", luab_Controller_is_jumping},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Controller_funcs[] = {
    {"new", luab_Controller_new},
    {NULL, NULL}
};

int luaopen_dabes_body(lua_State *L) {
    luaL_newmetatable(L, luab_Controller_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Controller_meths, 0);
    luaL_newlib(L, luab_Controller_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Controller_lib);

    return 1;
}
