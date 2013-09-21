#ifndef __console_bindings_h
#define __console_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "engine.h"

extern const char *luab_Console_lib;

int luaopen_dabes_console(lua_State *L);

#endif
