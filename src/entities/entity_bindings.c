#include "entity_bindings.h"

const char *luab_Entity_lib = "dab_entity";
const char *luab_Entity_metatable = "DaBes.entity";

Scripting_num_getter(Entity, alpha);
Scripting_num_setter(Entity, alpha);

static const struct luaL_Reg luab_Entity_meths[] = {
    {"__gc", luab_Entity_close},
    {"get_controller", luab_Entity_get_controller},
    {"set_controller", luab_Entity_set_controller},
    {"get_sprite", luab_Entity_get_sprite},
    {"set_sprite", luab_Entity_set_sprite},
    {"get_body", luab_Entity_get_body},
    {"set_body", luab_Entity_set_body},
    {"get_alpha", luab_Entity_get_alpha},
    {"set_alpha", luab_Entity_set_alpha},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Entity_funcs[] = {
    {"new", luab_Entity_new},
    {NULL, NULL}
};

int luaopen_dabes_body(lua_State *L) {
    luaL_newmetatable(L, luab_Entity_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Entity_meths, 0);
    luaL_newlib(L, luab_Entity_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Entity_lib);

    return 1;
}
