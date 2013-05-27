#ifndef __parallax_bindings_h
#define __parallax_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "parallax.h"

extern const char *luab_ParallaxLayer_lib;
extern const char *luab_ParallaxLayer_metatable;
typedef Scripting_userdata_for(ParallaxLayer) ParallaxLayer_userdata;
Scripting_caster_for(ParallaxLayer, luaL_toparallaxlayer);

extern const char *luab_Parallax_lib;
extern const char *luab_Parallax_metatable;
typedef Scripting_userdata_for(Parallax) Parallax_userdata;
Scripting_caster_for(Parallax, luaL_toparallax);

int luaopen_dabes_parallax(lua_State *L);

#endif
