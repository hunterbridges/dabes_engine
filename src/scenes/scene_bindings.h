#ifndef __scene_bindings_h
#define __scene_bindings_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "scene.h"

typedef struct Scene_userdata {
    lua_State *L;
    Scene *scene;
} Scene_userdata;

int luaopen_dabes_scene(lua_State *L);
Scene *luaL_get_current_scene(lua_State *L);

#pragma mark Script Bindings
int Scene_init(Scene *scene, Engine *engine);
int Scene_configure(Scene *scene, Engine *engine);

#endif
