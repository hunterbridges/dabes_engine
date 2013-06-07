#include "sensor_bindings.h"
#include "../core/engine.h"

const char *luab_Sensor_lib = "dab_sensor";
const char *luab_Sensor_metatable = "DaBes.sensor";

int luab_Sensor_new(lua_State *L) {
    Sensor_userdata *ud = NULL;

    ud = lua_newuserdata(L, sizeof(Sensor_userdata));
    check(ud != NULL, "Could not make Sensor userdata");

    luaL_getmetatable(L, luab_Sensor_metatable);
    lua_setmetatable(L, -2);

    float w = lua_tonumber(L, 1);
    float h = lua_tonumber(L, 2);
    VPoint offset = luaL_tovpoint(L, 3);
    Sensor *sensor = Sensor_create(w, h, offset);
    check(sensor != NULL, "Could not make sensor");
    luaL_register_ud(L, -1, (void **)&ud->p, sensor);

    return 1;
error:
    return 0;
}

Scripting_destroy_closer(Sensor);

Scripting_bool_getter(Sensor, on_static);

static const struct luaL_Reg luab_Sensor_meths[] = {
    {"__gc", luab_Sensor_close},
    {"get_on_static", luab_Sensor_get_on_static},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Sensor_funcs[] = {
    {"new", luab_Sensor_new},
    {NULL, NULL}
};

int luaopen_dabes_sensor(lua_State *L) {
    luaL_newmetatable(L, luab_Sensor_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Sensor_meths, 0);
    luaL_newlib(L, luab_Sensor_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Sensor_lib);

    return 1;
}
