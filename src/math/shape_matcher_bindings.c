#include "shape_matcher_bindings.h"

const char *luab_Shape_lib = "dab_shape";
const char *luab_Shape_metatable = "DaBes.shape";

int luab_Shape_new(lua_State *L) {
    Shape_userdata *ud = NULL;

    const char *name = lua_tostring(L, 1);

    int c = lua_gettop(L) - 1;
    ShapeSegment segments[c];

    ud = lua_newuserdata(L, sizeof(Shape_userdata));
    check(ud != NULL, "Could not make ShapeMatcher userdata");
    ud->p = NULL;

    luaL_getmetatable(L, luab_Shape_metatable);
    lua_setmetatable(L, -2);

    int i = 0;
    for (i = 0; i < c; i++) {
        int idx = i + 2;
        lua_getfield(L, idx, "length");
        float length = lua_tonumber(L, -1);
        lua_pop(L, 1);

        lua_getfield(L, idx, "angle");
        float angle = lua_tonumber(L, -1);
        lua_pop(L, 1);

        segments[i] = ShapeSegment_make(length, angle);
    }

    Shape *shape = Shape_create(segments, c, name);

    luaL_register_ud(L, -1, (void **)&ud->p, shape);
    return 1;
error:
    return 0;
}


Scripting_destroy_closer(Shape);
Scripting_string_getter(Shape, name);

static const struct luaL_Reg luab_Shape_meths[] = {
    {"__gc", luab_Shape_close},
    {"get_name", luab_Shape_get_name},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Shape_funcs[] = {
    {"new", luab_Shape_new},
    {NULL, NULL}
};

////////////////////////////////////////////////////////////////////////////////

const char *luab_ShapeMatcher_lib = "dab_shape_matcher";
const char *luab_ShapeMatcher_metatable = "DaBes.shape_matcher";

int luab_ShapeMatcher_new(lua_State *L) {
    ShapeMatcher_userdata *ud = NULL;

    int c = lua_gettop(L);

    Shape *shapes[c];

    ud = lua_newuserdata(L, sizeof(ShapeMatcher_userdata));
    check(ud != NULL, "Could not make ShapeMatcher userdata");
    ud->p = NULL;

    luaL_getmetatable(L, luab_ShapeMatcher_metatable);
    lua_setmetatable(L, -2);

    int i = 0;
    for (i = 0; i < c; i++) {
        Shape *shape = luaL_toshape(L, 1);
        shapes[i] = shape;
        lua_remove(L, 1);
    }

    ShapeMatcher *matcher = ShapeMatcher_create(shapes, c);

    luaL_register_ud(L, -1, (void **)&ud->p, matcher);
    return 1;
error:
    return 0;
}

Scripting_destroy_closer(ShapeMatcher);

Scripting_num_getter(ShapeMatcher, state);
Scripting_num_getter(ShapeMatcher, vertex_catch_tolerance);
Scripting_num_setter(ShapeMatcher, vertex_catch_tolerance);
Scripting_num_getter(ShapeMatcher, slop_tolerance);
Scripting_num_setter(ShapeMatcher, slop_tolerance);
Scripting_num_getter(ShapeMatcher, initial_segment_angle);
Scripting_num_setter(ShapeMatcher, initial_segment_angle);
Scripting_num_getter(ShapeMatcher, initial_segment_length);
Scripting_num_setter(ShapeMatcher, initial_segment_length);
Scripting_num_getter(ShapeMatcher, intended_convex_winding);
Scripting_num_setter(ShapeMatcher, intended_convex_winding);
Scripting_bool_getter(ShapeMatcher, debug_shapes);
Scripting_bool_setter(ShapeMatcher, debug_shapes);
Scripting_VVector4_getter(ShapeMatcher, debug_shape_color);
Scripting_VVector4_setter(ShapeMatcher, debug_shape_color);
Scripting_num_getter(ShapeMatcher, debug_shape_width);
Scripting_num_setter(ShapeMatcher, debug_shape_width);
Scripting_VVector4_getter(ShapeMatcher, dot_color);
Scripting_VVector4_setter(ShapeMatcher, dot_color);
Scripting_num_getter(ShapeMatcher, dot_width);
Scripting_num_setter(ShapeMatcher, dot_width);

static const struct luaL_Reg luab_ShapeMatcher_meths[] = {
    {"__gc", luab_ShapeMatcher_close},
    {"get_state", luab_ShapeMatcher_get_state},
    {"get_vertex_catch_tolerance",
        luab_ShapeMatcher_get_vertex_catch_tolerance},
    {"set_vertex_catch_tolerance",
        luab_ShapeMatcher_set_vertex_catch_tolerance},
    {"get_slop_tolerance",
        luab_ShapeMatcher_get_slop_tolerance},
    {"set_slop_tolerance",
        luab_ShapeMatcher_set_slop_tolerance},
    {"get_initial_segment_angle",
        luab_ShapeMatcher_get_initial_segment_angle},
    {"get_initial_segment_length",
        luab_ShapeMatcher_get_initial_segment_length},
    {"get_intended_convex_winding",
        luab_ShapeMatcher_get_intended_convex_winding},
    {"get_debug_shapes",
        luab_ShapeMatcher_get_debug_shapes},
    {"set_debug_shapes",
        luab_ShapeMatcher_set_debug_shapes},
    {"get_debug_shape_color",
        luab_ShapeMatcher_get_debug_shape_color},
    {"set_debug_shape_color",
        luab_ShapeMatcher_set_debug_shape_color},
    {"get_debug_shape_width",
        luab_ShapeMatcher_get_debug_shape_width},
    {"set_debug_shape_width",
        luab_ShapeMatcher_set_debug_shape_width},
    {"get_dot_color",
        luab_ShapeMatcher_get_dot_color},
    {"set_dot_color",
        luab_ShapeMatcher_set_dot_color},
    {"get_dot_width",
        luab_ShapeMatcher_get_dot_width},
    {"set_dot_width",
        luab_ShapeMatcher_set_dot_width},
    {NULL, NULL}
};

static const struct luaL_Reg luab_ShapeMatcher_funcs[] = {
    {"new", luab_ShapeMatcher_new},
    {NULL, NULL}
};

int luaopen_dabes_shape_matcher(lua_State *L) {
    luaL_newmetatable(L, luab_Shape_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Shape_meths, 0);
    luaL_newlib(L, luab_Shape_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Shape_lib);

    luaL_newmetatable(L, luab_ShapeMatcher_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_ShapeMatcher_meths, 0);
    luaL_newlib(L, luab_ShapeMatcher_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_ShapeMatcher_lib);

    return 1;
}
