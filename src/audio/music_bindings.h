#ifndef __music_bindings_h
#define __music_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "../scripting/scripting.h"
#include "music.h"

extern const char *luab_Music_lib;
extern const char *luab_Music_metatable;

typedef Scripting_userdata_for(Music) Music_userdata;

Scripting_caster_for(Music, luaL_tomusic);

int luaopen_dabes_music(lua_State *L);

#endif
