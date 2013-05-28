#include "body_bindings.h"

const char *luab_Body_lib = "dab_body";
const char *luab_Body_metatable = "DaBes.body";

static const struct luaL_Reg luab_Body_meths[] = {
    {"__gc", luab_Body_close},
    {"apply_force", luab_Body_apply_force},
    {"get_pos", luab_Body_get_pos},
    {"set_pos", luab_Body_set_pos},
    {"get_velo", luab_Body_get_velo},
    {"set_velo", luab_Body_set_velo},
    {"get_force", luab_Body_get_force},
    {"set_force", luab_Body_set_force},
    {"get_angle", luab_Body_get_angle},
    {"set_angle", luab_Body_set_angle},
    {"get_friction", luab_Body_get_friction},
    {"set_friction", luab_Body_set_friction},
    {"get_mass", luab_Body_get_mass},
    {"set_mass", luab_Body_set_mass},
    {"get_can_rotate", luab_Body_get_can_rotate},
    {"set_can_rotate", luab_Body_set_can_rotate},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Body_funcs[] = {
    {"new", luab_Body_new},
    {NULL, NULL}
};

int luaopen_dabes_body(lua_State *L) {
    luaL_newmetatable(L, luab_Body_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Body_meths, 0);
    luaL_newlib(L, luab_Body_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Body_lib);

    return 1;
}
