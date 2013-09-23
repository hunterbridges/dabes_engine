#ifndef __recorder_bindings_h
#define __recorder_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "recorder.h"

extern const char *luab_Recorder_lib;
extern const char *luab_Recorder_metatable;

typedef Scripting_userdata_for(Recorder) Recorder_userdata;

Scripting_caster_for(Recorder, luaL_torecorder);

int luaopen_dabes_recorder(lua_State *L);

#endif
