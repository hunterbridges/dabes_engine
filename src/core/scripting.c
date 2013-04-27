#include "scripting.h"

const char *SCRIPTING_CL_ENTITY_CONFIG = "entity_config";
const char *SCRIPTING_CL_PARALLAX = "parallax";
const char *SCRIPTING_CL_PARALLAX_LAYER = "parallax_layer";

Scripting *Scripting_create(const char *boot_script) {
    Scripting *scripting = malloc(sizeof(Scripting));
    check(scripting != NULL, "Could not create scripting");

    scripting->L = luaL_newstate();
    lua_State *L = scripting->L;
    luaL_openlibs(scripting->L);
  
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
