#ifndef __scripting_h
#define __scripting_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"

extern const char *SCRIPTING_CL_ENTITY_CONFIG;
extern const char *SCRIPTING_CL_PARALLAX;
extern const char *SCRIPTING_CL_PARALLAX_LAYER;
extern const char *SCRIPTING_ENGINE_REGISTRY_KEY;

typedef struct Scripting {
    lua_State *L;
} Scripting;

struct Engine;
Scripting *Scripting_create(struct Engine *engine, const char *boot_script);
void Scripting_destroy(Scripting *scripting);
void Scripting_register_engine(Scripting *scripting, struct Engine *engine);
void Scripting_boot(Scripting *scripting);
int Scripting_call_hook(Scripting *scripting, void *bound, const char *fname);

void luaL_register_ud(lua_State *L, int ud_idx, void **ud_prop, void *val);
int luaL_lookup_ud(lua_State *L, void *val);
int luaL_lookup_instance(lua_State *L, void *val);
void luaL_createweaktable(lua_State *L);
int luab_register_instance(lua_State *L);
struct Engine *luaL_get_engine(lua_State *L);

#define Scripting_bail(L, MSG) { \
    fprintf(stderr, "%s: %s\n", MSG, lua_tostring(L, -1)); \
    return 0; \
}

#define Scripting_userdata_for(STYPE) \
struct STYPE ## _userdata { \
    lua_State *L; \
    STYPE *p; \
}

#define Scripting_caster_for(STYPE, FNAME) \
static inline STYPE *FNAME(lua_State *L, int narg) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) \
        luaL_checkudata(L, narg, luab_ ## STYPE ## _metatable); \
    check(ud != NULL, "Object at %d is not type %s", narg, #STYPE); \
    check(ud->p != NULL, "Userdata has no %s object", #STYPE); \
    return ud->p; \
error: \
    return NULL; \
}

#define Scripting_num_setter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _set_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_checkudata(L, 1, luab_ ## STYPE ## _metatable); \
    check(lua_isnumber(L, 2), \
            "Please provide a number to set "#STYPE"->"#SPROP); \
    lua_Number num = lua_tonumber(L, 2); \
    STYPE *s = ud->p; \
    printf("Setting ("#STYPE" %p)->"#SPROP": %f\n", s, num); \
    s->SPROP = num; \
    return 1; \
error: \
    return 0; \
}

#define Scripting_num_getter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _get_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_checkudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    lua_Number ret = s->SPROP; \
    printf("Getting ("#STYPE" %p)->SPROP: %f\n", s, ret); \
    lua_pushinteger(L, ret); \
    return 1; \
}

#define Scripting_bool_setter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _set_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_checkudata(L, 1, luab_ ## STYPE ## _metatable); \
    check(lua_isboolean(L, 2), \
            "Please provide a boolean to set "#STYPE"->"#SPROP); \
    int num = lua_toboolean(L, 2); \
    STYPE *s = ud->p; \
    printf("Setting ("#STYPE" %p)->"#SPROP": %d\n", s, num); \
    s->SPROP = num; \
    return 1; \
error: \
    return 0; \
}

#define Scripting_bool_getter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _get_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_checkudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    int ret = s->SPROP; \
    printf("Getting ("#STYPE" %p)->SPROP: %d\n", s, ret); \
    lua_pushboolean(L, ret); \
    return 1; \
}

#endif
