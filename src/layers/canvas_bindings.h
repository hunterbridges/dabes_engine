#ifndef __canvas_bindings_h
#define __canvas_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "canvas.h"

extern const char *luab_Canvas_lib;
extern const char *luab_Canvas_metatable;
typedef Scripting_userdata_for(Canvas) Canvas_userdata;
Scripting_caster_for(Canvas, luaL_tocanvas);

int luaopen_dabes_canvas(lua_State *L);


#endif
