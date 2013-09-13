#ifndef __shape_matcher_bindings_h
#define __shape_matcher_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "shape_matcher.h"

extern const char *luab_Shape_lib;
extern const char *luab_Shape_metatable;
typedef Scripting_userdata_for(Shape) Shape_userdata;
Scripting_caster_for(Shape, luaL_toshape);

extern const char *luab_ShapeMatcher_lib;
extern const char *luab_ShapeMatcher_metatable;
typedef Scripting_userdata_for(ShapeMatcher) ShapeMatcher_userdata;
Scripting_caster_for(ShapeMatcher, luaL_toshapematcher);

int luaopen_dabes_shape_matcher(lua_State *L);

#endif
