#ifndef __body_bindings_h
#define __body_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "body.h"

extern const char *luab_Body_lib;
extern const char *luab_Body_metatable;
typedef Scripting_userdata_for(Body) Body_userdata;
Scripting_caster_for(Body, luaL_tobody);

int luaopen_dabes_body(lua_State *L);

#endif
