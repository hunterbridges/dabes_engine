#ifndef __sprite_bindings_h
#define __sprite_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "sprite.h"

extern const char *luab_SpriteAnimation_lib;
extern const char *luab_SpriteAnimation_metatable;
typedef Scripting_userdata_for(SpriteAnimation) SpriteAnimation_userdata;
Scripting_caster_for(SpriteAnimation, luaL_tospriteanimation);

extern const char *luab_Sprite_lib;
extern const char *luab_Sprite_metatable;
typedef Scripting_userdata_for(Sprite) Sprite_userdata;
Scripting_caster_for(Sprite, luaL_toparallax);

int luaopen_dabes_sprite(lua_State *L);

#endif
