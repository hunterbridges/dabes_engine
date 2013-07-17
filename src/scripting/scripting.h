#ifndef __scripting_h
#define __scripting_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"
#include "binding_macros.h"
#include "../math/vpoint.h"
#include "../math/vmatrix.h"

extern const char *SCRIPTING_CL_ENTITY_CONFIG;
extern const char *SCRIPTING_CL_PARALLAX;
extern const char *SCRIPTING_CL_PARALLAX_LAYER;
extern const char *SCRIPTING_ENGINE_REGISTRY_KEY;

typedef void (*Scripting_error_cb)(const char *error);

typedef struct Scripting {
    lua_State *L;
    Scripting_error_cb error_callback;
    Scripting_error_cb panic_callback;
} Scripting;

struct Engine;
Scripting *Scripting_create(struct Engine *engine, const char *boot_script);
void Scripting_destroy(Scripting *scripting);
void Scripting_register_engine(Scripting *scripting, struct Engine *engine);
void Scripting_boot(Scripting *scripting);
void Scripting_update_paths(Scripting *scripting, struct Engine *engine);

int Scripting_call_hook(Scripting *scripting, void *bound, const char *fname);
void *Scripting_ud_return_hook(Scripting *scripting, void *bound,
        const char *fname);

void luaL_register_ud(lua_State *L, int ud_idx, void **ud_prop, void *val);
int luaL_lookup_ud(lua_State *L, void *val);
int luaL_lookup_instance(lua_State *L, void *val);
void luaL_createweakweaktable(lua_State *L);
void luaL_createweakstrongtable(lua_State *L);
void luaL_createstrongweaktable(lua_State *L);
int luab_register_instance(lua_State *L);
struct Engine *luaL_get_engine(lua_State *L);
int luaL_unpack_exact (lua_State *L, int count);

VPoint luaL_tovpoint(lua_State *L, int idx);
int luaL_pushvpoint(lua_State *L, VPoint point);

VVector4 luaL_tovvector4(lua_State *L, int idx);

#define Scripting_bail(L, MSG) { \
    fprintf(stderr, "%s: %s\n", MSG, lua_tostring(L, -1)); \
    return 0; \
}

#endif
