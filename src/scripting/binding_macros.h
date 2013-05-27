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
        luaL_checkudata(L, narg, luab_ ## STYPE ## _metatable); \
    check(ud != NULL, "Object at %d is not type %s", narg, #STYPE); \
    check(ud->p != NULL, "Userdata has no %s object", #STYPE); \
    return ud->p; \
error: \
    return NULL; \
}

// Property accessor generation, basic types
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

// Property synthesis, complex types
#define Scripting_VPoint_getter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _get_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_checkudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    printf("Getting (" #STYPE " %p)->" #SPROP ": {%f, %f}\n", s, \
            s->SPROP.x, s->SPROP.y); \
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
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_checkudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    check(luaL_unpack_exact(L, 2), \
            "Please provide 2 numbers to set " #STYPE "->" #SPROP ); \
    VPoint point = {lua_tonumber(L, -2), lua_tonumber(L, -1)}; \
    lua_pop(L, 2); \
    printf("Setting (" #STYPE " %p)->" #SPROP ": {%f, %f}\n", s, \
            point.x, point.y); \
    s->SPROP = point; \
    return 1; \
error: \
    return 0; \
}

#define Scripting_GfxUVertex_getter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _get_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_checkudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    printf("Getting (" #STYPE " %p)->" #SPROP ": {%f, %f, %f, %f}\n", s, \
            s->SPROP.raw[0], s->SPROP.raw[1], s->SPROP.raw[2], s->SPROP.raw[3]); \
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

#define Scripting_GfxUVertex_setter(STYPE, SPROP) \
static inline int luab_ ## STYPE ## _set_ ## SPROP(lua_State *L) { \
    STYPE ## _userdata *ud = (STYPE ## _userdata *) luaL_checkudata(L, 1, luab_ ## STYPE ## _metatable); \
    STYPE *s = ud->p; \
    check(luaL_unpack_exact(L, 4), \
            "Please provide 4 numbers to set " #STYPE "->" #SPROP ); \
    GfxUVertex vertex = {.raw = {lua_tonumber(L, -4), lua_tonumber(L, -3), \
        lua_tonumber(L, -2), lua_tonumber(L, -1)}}; \
    lua_pop(L, 4); \
    printf("Setting (" #STYPE " %p)->" #SPROP ": {%f, %f, %f, %f}\n", s, \
            vertex.raw[0], vertex.raw[1], vertex.raw[2], vertex.raw[3]); \
    s->SPROP = vertex; \
    return 1; \
error: \
    return 0; \
}

#endif
