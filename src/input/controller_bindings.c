#include "controller_bindings.h"
#include "../core/engine.h"

const char *luab_Controller_lib = "dab_controller";
const char *luab_Controller_metatable = "DaBes.controller";

int luab_Controller_new(lua_State *L) {
    Controller_userdata *ud = NULL;
    Engine *engine = luaL_get_engine(L);

    ud = lua_newuserdata(L, sizeof(Controller_userdata));
    check(ud != NULL, "Could not make Controller userdata");
    ud->p = NULL;
  
    luaL_getmetatable(L, luab_Controller_metatable);
    lua_setmetatable(L, -2);

    int idx = (int)lua_tointeger(L, 1) - 1;
    Controller *controller = engine->input->controllers[idx];
    luaL_register_ud(L, -1, (void **)&ud->p, controller);
    return 1;
error:
    return 0;
}

Scripting_null_closer(Controller);

int luab_Controller_is_up(lua_State *L) {
    Controller *controller = luaL_tocontroller(L, 1);
    check(controller != NULL, "Controller required");
    int is = (controller->dpad & CONTROLLER_DPAD_UP) > 0;
    lua_pushboolean(L, is);

    return 1;
error:
    return 0;
}

int luab_Controller_is_down(lua_State *L) {
    Controller *controller = luaL_tocontroller(L, 1);
    check(controller != NULL, "Controller required");
    int is = (controller->dpad & CONTROLLER_DPAD_DOWN) > 0;
    lua_pushboolean(L, is);

    return 1;
error:
    return 0;
}

int luab_Controller_is_left(lua_State *L) {
    Controller *controller = luaL_tocontroller(L, 1);
    check(controller != NULL, "Controller required");
    int is = (controller->dpad & CONTROLLER_DPAD_LEFT) > 0;
    lua_pushboolean(L, is);

    return 1;
error:
    return 0;
}

int luab_Controller_is_right(lua_State *L) {
    Controller *controller = luaL_tocontroller(L, 1);
    check(controller != NULL, "Controller required");
    int is = (controller->dpad & CONTROLLER_DPAD_RIGHT) > 0;
    lua_pushboolean(L, is);

    return 1;
error:
    return 0;
}

int luab_Controller_is_jumping(lua_State *L) {
    Controller *controller = luaL_tocontroller(L, 1);
    check(controller != NULL, "Controller required");
    lua_pushboolean(L, controller->jump);

    return 1;
error:
    return 0;
}

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

int luaopen_dabes_controller(lua_State *L) {
    luaL_newmetatable(L, luab_Controller_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Controller_meths, 0);
    luaL_newlib(L, luab_Controller_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Controller_lib);

    return 1;
}
