#include "scripting.h"

#include "../core/engine.h"
#include "../scenes/scene.h"

// Import all the bindings so we can load them into our scripting env.
#include "../audio/music_bindings.h"
#include "../input/controller_bindings.h"
#include "../entities/body_bindings.h"
#include "../entities/entity_bindings.h"
#include "../entities/sensor_bindings.h"
#include "../scenes/scene_bindings.h"
#include "../graphics/parallax_bindings.h"
#include "../graphics/sprite_bindings.h"

const char *SCRIPTING_CL_ENTITY_CONFIG = "entity_config";
const char *SCRIPTING_CL_PARALLAX = "parallax";
const char *SCRIPTING_CL_PARALLAX_LAYER = "parallax_layer";
const char *SCRIPTING_ENGINE_REGISTRY_KEY = "dabes_engine";
const char *SCRIPTING_POINTER_MAP = "pointermap";
const char *SCRIPTING_INSTANCE_MAP = "instances";

void Scripting_load_engine_libs(Scripting *scripting) {
    luaopen_dabes_music(scripting->L);
    luaopen_dabes_controller(scripting->L);
    luaopen_dabes_body(scripting->L);
    luaopen_dabes_entity(scripting->L);
    luaopen_dabes_sensor(scripting->L);
    luaopen_dabes_scene(scripting->L);
    luaopen_dabes_parallax(scripting->L);
    luaopen_dabes_sprite(scripting->L);
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
    lua_pushstring(scripting->L, resource_path("scripts/?.lua"));
    lua_setfield(scripting->L, -2, "path");
    lua_pop(scripting->L, 1);

    // The pointer map is keyed by C object pointers and contains
    // userdata objects.
    luaL_createweakweaktable(L);
    lua_setglobal(L, SCRIPTING_POINTER_MAP);

    // The instance map is keyed by userdata objects and contains
    // the Lua BoundObject instances.
    luaL_createweakweaktable(L);
    lua_setglobal(L, SCRIPTING_INSTANCE_MAP);

    lua_pushcfunction(L, luab_register_instance);
    lua_setglobal(L, "dab_registerinstance");

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

void Scripting_boot(Scripting *scripting) {
    lua_State *L = scripting->L;
    lua_getglobal(L, "boot");
    int result = lua_pcall(L, 0, 0, 0);
    if (result != 0) {
        debug("Error running boot():\n    %s", lua_tostring(L, -1));
    }
}

int Scripting_call_hook(Scripting *scripting, void *bound, const char *fname) {
    check(scripting != NULL, "No scripting to call hook in");
    check(bound != NULL, "No bound object to call hook on");
    lua_State *L = scripting->L;

    int result = luaL_lookup_instance(L, bound);
    if (!result) return 0;

    lua_getfield(L, -1, fname);
    lua_pushvalue(L, -2);
    result = lua_pcall(L, 1, 0, 0);
    if (result != 0) {
        debug("Error in %p %s hook,\n    %s", bound, fname, lua_tostring(L, -1));
        return 0;
    }
    lua_pop(L, 1);

    return 1;
error:
    return 0;
}

void *Scripting_ud_return_hook(Scripting *scripting, void *bound,
        const char *fname) {
    check(scripting != NULL, "No scripting to call hook in");
    check(bound != NULL, "No bound object to call hook on");
    lua_State *L = scripting->L;

    int result = luaL_lookup_instance(L, bound);
    if (!result) return NULL;

    lua_getfield(L, -1, fname);
    lua_pushvalue(L, -2);
    result = lua_pcall(L, 1, 0, 1);
    if (result != 0) {
        debug("Error in %p %s hook,\n    %s", bound, fname, lua_tostring(L, -1));
        return NULL;
    }
    void *ret = lua_touserdata(L, -1);
    lua_pop(L, 2);

    return ret;
error:
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////

void luaL_register_ud(lua_State *L, int ud_idx, void **ud_prop, void *val) {
    int top = lua_gettop(L);

    lua_pushvalue(L, ud_idx);
    lua_getglobal(L, SCRIPTING_POINTER_MAP);
    lua_pushlightuserdata(L, val);
    lua_pushvalue(L, -3);
    lua_settable(L, -3);
    lua_pop(L, 2);

    // Assign the pointer val to the destination property on the userdata.
    *ud_prop = val;

    int newtop = lua_gettop(L);
    check(top == newtop, "Left stack dirty (%d, should be %d)", newtop, top);
    return;
error:
    return;
}

int luaL_lookup_ud(lua_State *L, void *val) {
    int top = lua_gettop(L);
    lua_getglobal(L, SCRIPTING_POINTER_MAP);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return 0;
    }

    lua_pushlightuserdata(L, val);
    lua_gettable(L, -2);
    lua_remove(L, -2); // Remove pointer map

    int newtop = lua_gettop(L);
    check(newtop - top == 1, "Left stack dirty (%d, should be %d)", newtop,
            top + 1);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return 0;
    }

    return 1;
error:
    return 0;
}

int luaL_lookup_instance(lua_State *L, void *val) {
    int top = lua_gettop(L);
    check(luaL_lookup_ud(L, val),
            "Couldn't find Userdata in pointer map");
    lua_getglobal(L, SCRIPTING_INSTANCE_MAP);
    lua_pushvalue(L, -2);
    lua_gettable(L, -2);
    lua_remove(L, -2); // Remove instance map
    lua_remove(L, -2); // Remove userdata

    // Now the instance should be on the stack
    int newtop = lua_gettop(L);
    check(newtop - top == 1, "Left stack dirty (%d, should be %d)", newtop,
            top + 1);
    if (lua_isnil(L, -1)) {
        lua_pop(L, 1);
        return 0;
    }

    return 1;
error:
    return 0;
}

int luab_register_instance(lua_State *L) {
    lua_getglobal(L, SCRIPTING_INSTANCE_MAP);
    lua_pushvalue(L, -3); // Key (userdata)
    lua_pushvalue(L, -3); // Val (instance)
    lua_settable(L, -3);
    lua_pop(L, 3);
    return 1;
}

void luaL_createweakweaktable(lua_State *L) {
    lua_newtable(L);

    lua_newtable(L); // metatable
    lua_pushstring(L, "kv");
    lua_setfield(L, -2, "__mode");

    lua_setmetatable(L, -2);
}

void luaL_createweakstrongtable(lua_State *L) {
    lua_newtable(L);

    lua_newtable(L); // metatable
    lua_pushstring(L, "k");
    lua_setfield(L, -2, "__mode");

    lua_setmetatable(L, -2);
}

void luaL_createstrongweaktable(lua_State *L) {
    lua_newtable(L);

    lua_newtable(L); // metatable
    lua_pushstring(L, "v");
    lua_setfield(L, -2, "__mode");

    lua_setmetatable(L, -2);
}

Engine *luaL_get_engine(lua_State *L) {
    lua_pushlightuserdata(L, (void *)SCRIPTING_ENGINE_REGISTRY_KEY);
    lua_gettable(L, LUA_REGISTRYINDEX);
    Engine *engine = (Engine *)lua_topointer(L, -1);
    lua_pop(L, 1);
    return engine;
}

// This one I literally copied and pasted from Lua and added the count check.
int luaL_unpack_exact (lua_State *L, int count) {
  int i, e, n;

  int tablepos = lua_gettop(L);
  luaL_checktype(L, -1, LUA_TTABLE);
  i = luaL_optint(L, tablepos + 1, 1);
  e = luaL_opt(L, luaL_checkint, tablepos + 2, luaL_len(L, tablepos));
  if (i > e) return 0;  /* empty range */
  n = e - i + 1;  /* number of elements */
  if (n <= 0 || !lua_checkstack(L, n))  /* n <= 0 means arith. overflow */
    return luaL_error(L, "too many results to unpack");
  if (n != count)
      return luaL_error(L, "wrong number of results to unpack (%d, expected %d)",
              n, count);
  lua_rawgeti(L, tablepos, i);  /* push arg[i] (avoiding overflow problems) */
  while (i++ < e)  /* push arg[i + 1...e] */
    lua_rawgeti(L, tablepos, i);
  return n;
}

VPoint luaL_tovpoint(lua_State *L, int idx) {
    lua_pushvalue(L, idx);
    check(luaL_unpack_exact(L, 2),
            "Please provide 2 numbers to get a VPoint. Returning VPointZero.");
    VPoint point = {lua_tonumber(L, -2), lua_tonumber(L, -1)};
    lua_pop(L, 3);
    return point;
error:
    return VPointZero;
}

int luaL_pushvpoint(lua_State *L, VPoint point) {
    lua_newtable(L);
    lua_pushinteger(L, 1);
    lua_pushnumber(L, point.x);
    lua_settable(L, -3);
    lua_pushinteger(L, 2);
    lua_pushnumber(L, point.y);
    lua_settable(L, -3);
    return 1;
}
