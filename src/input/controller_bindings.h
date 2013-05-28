#ifndef __controller_bindings_h
#define __controller_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "controller.h"

extern const char *luab_Controller_lib;
extern const char *luab_Controller_metatable;
typedef Scripting_userdata_for(Controller) Controller_userdata;
Scripting_caster_for(Controller, luaL_tocontroller);

int luaopen_dabes_controller(lua_State *L);

#endif
