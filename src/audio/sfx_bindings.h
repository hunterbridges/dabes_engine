#ifndef __sfx_bindings
#define __sfx_bindings
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "sfx.h"

extern const char *luab_Sfx_lib;
extern const char *luab_Sfx_metatable;

typedef Scripting_userdata_for(Sfx) Sfx_userdata;

Scripting_caster_for(Sfx, luaL_tosfx);

int luaopen_dabes_sfx(lua_State *L);

#endif
