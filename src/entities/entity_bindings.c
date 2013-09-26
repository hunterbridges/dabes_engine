#include "entity_bindings.h"
#include "../entities/body_bindings.h"
#include "../graphics/sprite_bindings.h"
#include "../input/controller_bindings.h"

const char *luab_Entity_lib = "dab_entity";
const char *luab_Entity_metatable = "DaBes.entity";

Scripting_num_getter(Entity, alpha);
Scripting_num_setter(Entity, alpha);

int luab_Entity_new(lua_State *L) {
    Entity_userdata *ud = NULL;
    Engine *engine = luaL_get_engine(L);

    ud = lua_newuserdata(L, sizeof(Entity_userdata));
    check(ud != NULL, "Could not make Entity userdata");
    ud->p = NULL;

    luaL_getmetatable(L, luab_Entity_metatable);
    lua_setmetatable(L, -2);

    Entity *entity = Entity_create(engine);
    luaL_register_ud(L, -1, (void **)&ud->p, entity);
    return 1;
error:
    return 0;
}

int luab_Entity_close(lua_State *L) {
    Entity_userdata *ud = (Entity_userdata *)
        luaL_testudata(L, 1, luab_Entity_metatable);
    if (ud->p) {
        Entity_destroy(ud->p);
    }
    ud->p = NULL;
    return 0;
}

int luab_Entity_get_controller(lua_State *L) {
    Entity *entity = luaL_toentity(L, 1);
    check(entity != NULL, "Entity required");
    if (entity->controller == NULL) {
        lua_pushnil(L);
        return 1;
    }
    luaL_lookup_instance(L, entity->controller);

    return 1;
error:
    return 0;
}

int luab_Entity_set_controller(lua_State *L) {
    Entity *entity = luaL_toentity(L, 1);
    check(entity != NULL, "Entity required");
    lua_getfield(L, 2, "real");
    Controller *controller = luaL_tocontroller(L, -1);
    entity->controller = controller;
    lua_pop(L, 1);

    return 0;
error:
    return 0;
}

Scripting_bool_getter(Entity, auto_control);
Scripting_bool_setter(Entity, auto_control);
Scripting_bool_getter(Entity, force_keyframe);
Scripting_bool_setter(Entity, force_keyframe);

int luab_Entity_get_scene(lua_State *L) {
    Entity *entity = luaL_toentity(L, 1);
    check(entity != NULL, "Entity required");
    if (entity->scene == NULL) {
        lua_pushnil(L);
        return 1;
    }
    luaL_lookup_instance(L, entity->scene);

    return 1;
error:
    return 0;
}

int luab_Entity_get_sprite(lua_State *L) {
    Entity *entity = luaL_toentity(L, 1);
    check(entity != NULL, "Entity required");
    if (entity->sprite == NULL) {
        lua_pushnil(L);
        return 1;
    }
    luaL_lookup_instance(L, entity->sprite);

    return 1;
error:
    return 0;
}

int luab_Entity_set_sprite(lua_State *L) {
    Entity *entity = luaL_toentity(L, 1);
    check(entity != NULL, "Entity required");
    lua_getfield(L, 2, "real");
    Sprite *sprite = luaL_tosprite(L, -1);
    entity->sprite = sprite;
    lua_pop(L, 1);

    return 0;
error:
    return 0;
}

int luab_Entity_get_body(lua_State *L) {
    Entity *entity = luaL_toentity(L, 1);
    check(entity != NULL, "Entity required");
    if (entity->body == NULL) {
        lua_pushnil(L);
        return 1;
    }
    luaL_lookup_instance(L, entity->body);

    return 1;
error:
    return 0;
}

int luab_Entity_set_body(lua_State *L) {
    Entity *entity = luaL_toentity(L, 1);
    check(entity != NULL, "Entity required");
    lua_getfield(L, 2, "real");
    Body *body = luaL_tobody(L, -1);
    entity->body = body;
    lua_pop(L, 1);

    return 0;
error:
    return 0;
}

Scripting_num_getter(Entity, z_index);

int luab_Entity_set_z_index(lua_State *L) {
    int z_index = lua_tonumber(L, 2);
    Entity *entity = luaL_toentity(L, 1);
    check(entity != NULL, "Entity required");
    Entity_set_z_index(entity, z_index);

    return 0;
error:
    return 0;
}

Scripting_VPoint_getter(Entity, center);

int luab_Entity_set_center(lua_State *L) {
    Entity *entity = luaL_toentity(L, 1);
    check(entity != NULL, "Entity required");
    VPoint pos = luaL_tovpoint(L, 2);
    Entity_set_center(entity, pos);

    return 0;
error:
    return 0;
}

Scripting_GfxSize_getter(Entity, size);

GfxSize luaL_togfxsize(lua_State *L, int idx) {
    lua_pushvalue(L, idx);
    check(luaL_unpack_exact(L, 2),
            "Please provide 2 numbers to get a GfxSize. Returning GfxSizeZero.");
    GfxSize size = {lua_tonumber(L, -2), lua_tonumber(L, -1)};
    lua_pop(L, 3);
    return size;
error:
    return GfxSizeZero;
}

int luab_Entity_set_size(lua_State *L) {
    Entity *entity = luaL_toentity(L, 1);
    check(entity != NULL, "Entity required");
    GfxSize size = luaL_togfxsize(L, 2);
    Entity_set_size(entity, size);

    return 0;
error:
    return 0;
}

static const struct luaL_Reg luab_Entity_meths[] = {
    {"__gc", luab_Entity_close},
    {"get_controller", luab_Entity_get_controller},
    {"set_controller", luab_Entity_set_controller},
    {"get_auto_control", luab_Entity_get_auto_control},
    {"set_auto_control", luab_Entity_set_auto_control},
    {"get_force_keyframe", luab_Entity_get_force_keyframe},
    {"set_force_keyframe", luab_Entity_set_force_keyframe},
    {"get_scene", luab_Entity_get_scene},
    {"get_sprite", luab_Entity_get_sprite},
    {"set_sprite", luab_Entity_set_sprite},
    {"get_body", luab_Entity_get_body},
    {"set_body", luab_Entity_set_body},
    {"get_alpha", luab_Entity_get_alpha},
    {"set_alpha", luab_Entity_set_alpha},
    {"get_z_index", luab_Entity_get_z_index},
    {"set_z_index", luab_Entity_set_z_index},
    {"get_center", luab_Entity_get_center},
    {"set_center", luab_Entity_set_center},
    {"get_size", luab_Entity_get_size},
    {"set_size", luab_Entity_set_size},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Entity_funcs[] = {
    {"new", luab_Entity_new},
    {NULL, NULL}
};

int luaopen_dabes_entity(lua_State *L) {
    luaL_newmetatable(L, luab_Entity_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Entity_meths, 0);
    luaL_newlib(L, luab_Entity_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Entity_lib);

    return 1;
}
