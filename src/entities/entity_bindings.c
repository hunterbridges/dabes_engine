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

    ud = lua_newuserdata(L, sizeof(Entity_userdata));
    check(ud != NULL, "Could not make Entity userdata");

    luaL_getmetatable(L, luab_Entity_metatable);
    lua_setmetatable(L, -2);

    Entity *entity = Entity_create();
    luaL_register_ud(L, -1, (void **)&ud->p, entity);
    return 1;
error:
    return 0;
}

int luab_Entity_close(lua_State *L) {
    Entity_userdata *ud = (Entity_userdata *)
        luaL_checkudata(L, 1, luab_Entity_metatable);
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


static const struct luaL_Reg luab_Entity_meths[] = {
    {"__gc", luab_Entity_close},
    {"get_controller", luab_Entity_get_controller},
    {"set_controller", luab_Entity_set_controller},
    {"get_sprite", luab_Entity_get_sprite},
    {"set_sprite", luab_Entity_set_sprite},
    {"get_body", luab_Entity_get_body},
    {"set_body", luab_Entity_set_body},
    {"get_alpha", luab_Entity_get_alpha},
    {"set_alpha", luab_Entity_set_alpha},
    {"get_z_index", luab_Entity_get_z_index},
    {"set_z_index", luab_Entity_set_z_index},
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
