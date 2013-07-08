#ifndef __engine_bindings_h
#define __engine_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "engine.h"

extern const char *luab_Engine_lib;

int luaopen_dabes_engine(lua_State *L);

#endif

