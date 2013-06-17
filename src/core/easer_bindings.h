#ifndef __easer_bindings_h
#define __easer_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "easer.h"

extern const char *luab_Easer_lib;
extern const char *luab_Easer_metatable;
typedef Scripting_userdata_for(Easer) Easer_userdata;
Scripting_caster_for(Easer, luaL_toeaser);

int luaopen_dabes_easer(lua_State *L);

#endif
