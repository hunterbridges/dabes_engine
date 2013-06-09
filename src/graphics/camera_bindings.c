#include "camera_bindings.h"
#include "../scenes/scene_bindings.h"
#include "../entities/entity_bindings.h"

const char *luab_Camera_lib = "dab_camera";
const char *luab_Camera_metatable = "DaBes.camera";

int luab_Camera_new(lua_State *L) {
    Camera_userdata *ud = NULL;

    ud = lua_newuserdata(L, sizeof(Camera_userdata));
    check(ud != NULL, "Could not make Camera userdata");

    luaL_getmetatable(L, luab_Camera_metatable);
    lua_setmetatable(L, -2);

    Scene *scene = luaL_toscene(L, 1);

    Camera *camera = scene->camera;
    luaL_register_ud(L, -1, (void **)&ud->p, camera);
    return 1;
error:
    return 0;
}

Scripting_null_closer(Camera);

int luab_Camera_track_entities(lua_State *L) {
    Camera *camera = luaL_tocamera(L, 1);
    check(camera != NULL, "Camera required");
    lua_remove(L, 1);
    int num_entities = lua_gettop(L);
    if (num_entities > 0) {
        Entity *entities[num_entities];
        int i = 0;
        for (i = 0; i < num_entities; i++) {
            entities[i] = luaL_toentity(L, 1);
            lua_remove(L, 1);
        }
        Camera_track_entities(camera, num_entities, entities);
    } else {
        Camera_track_entities(camera, num_entities, NULL);
    }

    return 0;
error:
    return 0;
}

Scripting_bool_getter(Camera, snap_to_scene);
Scripting_bool_setter(Camera, snap_to_scene);
Scripting_num_getter(Camera, max_scale);
Scripting_num_setter(Camera, max_scale);
Scripting_num_getter(Camera, min_scale);
Scripting_num_setter(Camera, min_scale);
Scripting_num_getter(Camera, scale);
Scripting_num_setter(Camera, scale);
Scripting_num_getter(Camera, rotation_radians);
Scripting_num_setter(Camera, rotation_radians);
Scripting_VPoint_getter(Camera, translation);
Scripting_VPoint_setter(Camera, translation);
Scripting_VPoint_getter(Camera, focal);
Scripting_VPoint_setter(Camera, focal);

static const struct luaL_Reg luab_Camera_meths[] = {
    {"__gc", luab_Camera_close},
    {"track_entities", luab_Camera_track_entities},
    {"get_snap_to_scene", luab_Camera_get_snap_to_scene},
    {"set_snap_to_scene", luab_Camera_set_snap_to_scene},
    {"get_max_scale", luab_Camera_get_max_scale},
    {"set_max_scale", luab_Camera_set_max_scale},
    {"get_min_scale", luab_Camera_get_min_scale},
    {"set_min_scale", luab_Camera_set_min_scale},
    {"get_scale", luab_Camera_get_scale},
    {"set_scale", luab_Camera_set_scale},
    {"get_rotation", luab_Camera_get_rotation_radians},
    {"set_rotation", luab_Camera_set_rotation_radians},
    {"get_translation", luab_Camera_get_translation},
    {"set_translation", luab_Camera_set_translation},
    {"get_focal", luab_Camera_get_focal},
    {"set_focal", luab_Camera_set_focal},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Camera_funcs[] = {
    {"new", luab_Camera_new},
    {NULL, NULL}
};

int luaopen_dabes_camera(lua_State *L) {
    luaL_newmetatable(L, luab_Camera_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Camera_meths, 0);
    luaL_newlib(L, luab_Camera_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Camera_lib);

    return 1;
}
