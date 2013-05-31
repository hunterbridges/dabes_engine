#ifndef __scene_bindings_h
#define __scene_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "scene.h"

extern const char *luab_Scene_lib;
extern const char *luab_Scene_metatable;

typedef Scripting_userdata_for(Scene) Scene_userdata;

Scripting_caster_for(Scene, luaL_toscene);

int luaopen_dabes_scene(lua_State *L);
Scene *luaL_get_current_scene(lua_State *L);

#endif
