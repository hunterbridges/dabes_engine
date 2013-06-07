#ifndef __sensor_bindings_h
#define __sensor_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "sensor.h"

extern const char *luab_Sensor_lib;
extern const char *luab_Sensor_metatable;
typedef Scripting_userdata_for(Sensor) Sensor_userdata;
Scripting_caster_for(Sensor, luaL_tosensor);

int luaopen_dabes_sensor(lua_State *L);

#endif
