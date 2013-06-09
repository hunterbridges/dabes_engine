#ifndef __camera_bindings_h
#define __camera_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "camera.h"

extern const char *luab_Camera_lib;
extern const char *luab_Camera_metatable;
typedef Scripting_userdata_for(Camera) Camera_userdata;
Scripting_caster_for(Camera, luaL_tocamera);

int luaopen_dabes_camera(lua_State *L);

#endif
