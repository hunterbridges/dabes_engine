#ifndef __overlay_bindings_h
#define __overlay_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "overlay.h"

extern const char *luab_Overlay_lib;
extern const char *luab_Overlay_metatable;
typedef Scripting_userdata_for(Overlay) Overlay_userdata;
Scripting_caster_for(Overlay, luaL_tooverlay);

int luaopen_dabes_overlay(lua_State *L);


#endif
