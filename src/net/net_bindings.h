#ifndef __net_bindings_h
#define __net_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "net.h"

extern const char *luab_Net_lib;
extern const char *luab_Net_metatable;
typedef Scripting_userdata_for(Net) Net_userdata;
Scripting_caster_for(Net, luaL_tonet);

int luaopen_dabes_net(lua_State *L);

#endif
