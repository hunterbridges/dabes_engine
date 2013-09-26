#ifndef __binding_macros_h
#define __binding_macros_h

// Defining Types
#define Scripting_userdata_for(STYPE) \
struct STYPE ## _userdata { \
    lua_State *L; \
    STYPE *p; \
}

#define Scripting_caster_for(STYPE, FNAME) \
static inline STYPE *FNAME(lua_State *L, int narg) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) \
        luaL_testudata(L, narg, luab_ ## STYPE ## _metatable); \
    check(ud != NULL, "Object at %d is not type %s", narg, #STYPE); \
    check(ud->p != NULL, "Userdata has no %s object", #STYPE); \
    return ud->p; \
error: \
    return NULL; \
}

#define Scripting_null_closer(STYPE) \
static inline int luab_ ## STYPE ## _close(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    ud->p = NULL; \
    return 0; \
}

#define Scripting_destroy_closer(STYPE) \
static inline int luab_ ## STYPE ## _close(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    if (ud->p) { \
        STYPE ## _destroy(ud->p); \
    } \
    ud->p = NULL; \
    return 0; \
}

// Property accessor generation, basic types
#define Scripting_num_setter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _set_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    check(lua_isnumber(L, 2), \
            "Please provide a number to set "#STYPE"->"#SPROP); \
    lua_Number num = lua_tonumber(L, 2); \
    STYPE *s = ud->p; \
    s->SPROP = num; \
    return 1; \
error: \
    return 0; \
}

#define Scripting_num_getter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _get_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    lua_Number ret = s->SPROP; \
    lua_pushnumber(L, ret); \
    return 1; \
}

#define Scripting_bool_setter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _set_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    check(lua_isboolean(L, 2), \
            "Please provide a boolean to set "#STYPE"->"#SPROP); \
    int num = lua_toboolean(L, 2); \
    STYPE *s = ud->p; \
    s->SPROP = num; \
    return 1; \
error: \
    return 0; \
}

#define Scripting_bool_getter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _get_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    int ret = s->SPROP; \
    lua_pushboolean(L, ret); \
    return 1; \
}

#define Scripting_string_setter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _set_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    check(lua_isstring(L, 2), \
            "Please provide a string to set "#STYPE"->"#SPROP); \
    char *string = lua_tostring(L, 2); \
    STYPE *s = ud->p; \
    s->SPROP = string; \
    return 1; \
error: \
    return 0; \
}

#define Scripting_string_getter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _get_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    char *ret = s->SPROP; \
    lua_pushstring(L, ret); \
    return 1; \
}

// Property synthesis, complex types
#define Scripting_VPoint_getter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _get_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    lua_newtable(L); \
    lua_pushinteger(L, 1); \
    lua_pushnumber(L, s->SPROP.x); \
    lua_settable(L, -3); \
    lua_pushinteger(L, 2); \
    lua_pushnumber(L, s->SPROP.y); \
    lua_settable(L, -3); \
    return 1; \
}

#define Scripting_VPoint_setter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _set_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    check(luaL_unpack_exact(L, 2), \
            "Please provide 2 numbers to set " #STYPE "->" #SPROP ); \
    VPoint point = {lua_tonumber(L, -2), lua_tonumber(L, -1)}; \
    lua_pop(L, 2); \
    s->SPROP = point; \
    return 1; \
error: \
    return 0; \
}

#define Scripting_VVector4_getter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _get_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    lua_newtable(L); \
    lua_pushinteger(L, 1); \
    lua_pushnumber(L, s->SPROP.raw[0]); \
    lua_settable(L, -3); \
    lua_pushinteger(L, 2); \
    lua_pushnumber(L, s->SPROP.raw[1]); \
    lua_settable(L, -3); \
    lua_pushinteger(L, 3); \
    lua_pushnumber(L, s->SPROP.raw[2]); \
    lua_settable(L, -3); \
    lua_pushinteger(L, 4); \
    lua_pushnumber(L, s->SPROP.raw[3]); \
    lua_settable(L, -3); \
    return 1; \
}

#define Scripting_VVector4_setter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _set_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    check(luaL_unpack_exact(L, 4), \
            "Please provide 4 numbers to set " #STYPE "->" #SPROP ); \
    VVector4 vertex = {.raw = {lua_tonumber(L, -4), lua_tonumber(L, -3), \
        lua_tonumber(L, -2), lua_tonumber(L, -1)}}; \
    lua_pop(L, 4); \
    s->SPROP = vertex; \
    return 1; \
error: \
    return 0; \
}

#define Scripting_GfxSize_getter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _get_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    lua_newtable(L); \
    lua_pushinteger(L, 1); \
    lua_pushnumber(L, s->SPROP.w); \
    lua_settable(L, -3); \
    lua_pushinteger(L, 2); \
    lua_pushnumber(L, s->SPROP.h); \
    lua_settable(L, -3); \
    return 1; \
}

#define Scripting_GfxSize_setter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _set_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_testudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    check(luaL_unpack_exact(L, 2), \
            "Please provide 2 numbers to set " #STYPE "->" #SPROP ); \
    GfxSize size = {lua_tonumber(L, -2), lua_tonumber(L, -1)}; \
    lua_pop(L, 2); \
    s->SPROP = size; \
    return 1; \
error: \
    return 0; \
}

#endif
