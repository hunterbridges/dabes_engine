#ifndef __net_match_bindings_h
#define __net_match_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "net_match.h"

extern const char *luab_NetMatch_lib;
extern const char *luab_NetMatch_metatable;
typedef Scripting_userdata_for(NetMatch) NetMatch_userdata;
Scripting_caster_for(NetMatch, luaL_tonetmatch);

int luaopen_dabes_netmatch(lua_State *L);

#endif
