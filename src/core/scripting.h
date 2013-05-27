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
int Scripting_test(Scripting *scripting);
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

#define Scripting_num_setter(FNAME, MTABLE, UDTYPE, UDPROP, STYPE, SPROP) \
static inline int FNAME(lua_State *L) { \
    UDTYPE *ud = (UDTYPE *) luaL_checkudata(L, 1, MTABLE); \
    check(lua_isnumber(L, 2), \
            "Please provide a number to set "#STYPE"->"#SPROP); \
    lua_Number num = lua_tonumber(L, 2); \
    STYPE *s = ud->UDPROP; \
    printf("Setting ("#STYPE" %p)->"#SPROP": %f\n", s, num); \
    s->SPROP = num; \
    return 1; \
error: \
    return 0; \
}

#define Scripting_num_getter(FNAME, MTABLE, UDTYPE, UDPROP, STYPE, SPROP) \
static inline int FNAME(lua_State *L) { \
    UDTYPE *ud = (UDTYPE *) luaL_checkudata(L, 1, MTABLE); \
    STYPE *s = ud->UDPROP; \
    lua_Number ret = s->SPROP; \
    printf("Getting ("#STYPE" %p)->SPROP: %f\n", s, ret); \
    lua_pushinteger(L, ret); \
    return 1; \
}

#define Scripting_bool_setter(FNAME, MTABLE, UDTYPE, UDPROP, STYPE, SPROP) \
static inline int FNAME(lua_State *L) { \
    UDTYPE *ud = (UDTYPE *) luaL_checkudata(L, 1, MTABLE); \
    check(lua_isboolean(L, 2), \
            "Please provide a boolean to set "#STYPE"->"#SPROP); \
    int num = lua_toboolean(L, 2); \
    STYPE *s = ud->UDPROP; \
    printf("Setting ("#STYPE" %p)->"#SPROP": %d\n", s, num); \
    s->SPROP = num; \
    return 1; \
error: \
    return 0; \
}

#define Scripting_bool_getter(FNAME, MTABLE, UDTYPE, UDPROP, STYPE, SPROP) \
static inline int FNAME(lua_State *L) { \
    UDTYPE *ud = (UDTYPE *) luaL_checkudata(L, 1, MTABLE); \
    STYPE *s = ud->UDPROP; \
    int ret = s->SPROP; \
    printf("Getting ("#STYPE" %p)->SPROP: %d\n", s, ret); \
    lua_pushboolean(L, ret); \
    return 1; \
}

#endif
