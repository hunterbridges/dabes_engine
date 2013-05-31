#ifndef __entity_bindings_h
#define __entity_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "entity.h"

extern const char *luab_Entity_lib;
extern const char *luab_Entity_metatable;
typedef Scripting_userdata_for(Entity) Entity_userdata;
Scripting_caster_for(Entity, luaL_toentity);

int luaopen_dabes_entity(lua_State *L);

#endif
