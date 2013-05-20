#include "scripting.h"

#include "engine.h"
#include "../scenes/scene.h"

// Import all the bindings so we can load them into our scripting env.
#include "../scenes/scene_bindings.h"

const char *SCRIPTING_CL_ENTITY_CONFIG = "entity_config";
const char *SCRIPTING_CL_PARALLAX = "parallax";
const char *SCRIPTING_CL_PARALLAX_LAYER = "parallax_layer";
const char *SCRIPTING_ENGINE_REGISTRY_KEY = "dabes_engine";


void Scripting_load_engine_libs(Scripting *scripting) {
    luaopen_dabes_scene(scripting->L);
}


Scripting *Scripting_create(struct Engine *engine, const char *boot_script) {
    Scripting *scripting = malloc(sizeof(Scripting));
    check(scripting != NULL, "Could not create scripting");

    scripting->L = luaL_newstate();
    lua_State *L = scripting->L;
    luaL_openlibs(scripting->L);

    Scripting_register_engine(scripting, engine);
    Scripting_load_engine_libs(scripting);

    lua_getglobal(scripting->L, "package");
    lua_pushstring(scripting->L, resource_path("media/scripts/?.lua"));
    lua_setfield(scripting->L, -2, "path");
    lua_pop(scripting->L, 1);

    int status = luaL_dofile(L, resource_path(boot_script));
    if (status) {
      fprintf(stderr, "Failed to run boot script: %s\n", lua_tostring(L, -1));
      free(scripting);
      return NULL;
    }

    return scripting;
error:
    if (scripting) free(scripting);
    return NULL;
}

void Scripting_destroy(Scripting *scripting) {
    check(scripting != NULL, "No scripting to destroy");
    lua_close(scripting->L);
    free(scripting);
    return;
error:
    return;
}

void Scripting_register_engine(Scripting *scripting, Engine *engine) {
    lua_State *L = scripting->L;
    lua_pushlightuserdata(L, (void *)SCRIPTING_ENGINE_REGISTRY_KEY);
    lua_pushlightuserdata(L, (void *)engine);
    lua_settable(L, LUA_REGISTRYINDEX);
}

Engine *luaL_get_engine(lua_State *L) {
    lua_pushlightuserdata(L, (void *)SCRIPTING_ENGINE_REGISTRY_KEY);
    lua_gettable(L, LUA_REGISTRYINDEX);
    Engine *engine = (Engine *)lua_topointer(L, -1);
    lua_pop(L, 1);
    return engine;
}

int Scripting_test(Scripting *scripting) {
    int status, result;
    int n = lua_gettop(scripting->L);
    status = luaL_dofile(scripting->L,
                         resource_path("media/scripts/hello.lua"));
    if (status) Scripting_bail(scripting->L, "Failed to load script");

    lua_getglobal(scripting->L, "test");
    lua_pushstring(scripting->L, "World");

    result = lua_pcall(scripting->L, 1, LUA_MULTRET, 0);
    if (result) Scripting_bail(scripting->L, "Failed to run script");
    int start = n + 1;
    n = lua_gettop(scripting->L) - n;

    int i = 1;
    for (i = 1; i <= n; i++) {
      const char *ret = lua_tostring(scripting->L, 1);
      printf("Script returned: %s\n", ret);
      lua_remove(scripting->L, start);
    }

    return 1;
}
