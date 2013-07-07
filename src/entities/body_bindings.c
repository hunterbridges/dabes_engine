#include "body_bindings.h"
#include "../core/engine.h"
#include "chipmunk_body.h"
#include "sensor_bindings.h"

const char *luab_Body_lib = "dab_body";
const char *luab_Body_metatable = "DaBes.body";

int luab_Body_new(lua_State *L) {
    Body_userdata *ud = NULL;

    const char *proto = lua_tostring(L, 1);
    float w = lua_tonumber(L, 2);
    float h = lua_tonumber(L, 3);
    float m = lua_tonumber(L, 4);
    int can_rotate = lua_toboolean(L, 5);

    ud = lua_newuserdata(L, sizeof(Body_userdata));
    check(ud != NULL, "Could not make Body userdata");
    ud->p = NULL;
  
    luaL_getmetatable(L, luab_Body_metatable);
    lua_setmetatable(L, -2);

    Body *body = NULL;
    int valid_proto = 0;
    if (streq(proto, "chipmunk")) {
        body = Body_create(ChipmunkBodyProto, w, h, m, can_rotate);
        valid_proto = 1;
    }
    check(valid_proto, "Invalid Body type: %s", proto);

    luaL_register_ud(L, -1, (void **)&ud->p, body);

    return 1;
error:
    return 0;
}

int luab_Body_close(lua_State *L) {
    Body_userdata *ud = (Body_userdata *)
        luaL_checkudata(L, 1, luab_Body_metatable);
    if (ud->p) {
        Body_destroy(ud->p);
    }
    ud->p = NULL;
    return 0;
}

int luab_Body_add_sensor(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    lua_getfield(L, 2, "real");
    Sensor *sensor = luaL_tosensor(L, -1);
    Body_add_sensor(body, sensor);

    return 0;
error:
    return 0;
}

int luab_Body_remove_sensor(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    lua_getfield(L, 2, "real");
    Sensor *sensor = luaL_tosensor(L, -1);
    Body_remove_sensor(body, sensor);
    lua_pop(L, 1);

    return 0;
error:
    return 0;
}

int luab_Body_apply_force(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    VPoint force = luaL_tovpoint(L, 2);
    VPoint offset = luaL_tovpoint(L, 3);
    body->_(apply_force)(body, force, offset);

    return 0;
error:
    return 0;
}

int luab_Body_set_hit_box(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    float w = lua_tonumber(L, 2);
    float h = lua_tonumber(L, 3);
    VPoint offset = luaL_tovpoint(L, 4);
    body->_(set_hit_box)(body, w, h, offset);

    return 0;
error:
    return 0;
}

int luab_Body_get_entity(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    if (body->state.entity == NULL) {
        lua_pushnil(L);
        return 1;
    }
    luaL_lookup_instance(L, body->state.entity);

    return 1;
error:
    return 0;
}

int luab_Body_get_pos(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    luaL_pushvpoint(L, body->_(get_pos)(body));

    return 1;
error:
    return 0;
}

int luab_Body_set_pos(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    VPoint pos = luaL_tovpoint(L, 2);
    body->_(set_pos)(body, pos);

    return 0;
error:
    return 0;
}

int luab_Body_get_velo(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    luaL_pushvpoint(L, body->_(get_velo)(body));

    return 1;
error:
    return 0;
}

int luab_Body_set_velo(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    VPoint velo = luaL_tovpoint(L, 2);
    body->_(set_velo)(body, velo);

    return 0;
error:
    return 0;
}

int luab_Body_get_force(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    luaL_pushvpoint(L, body->_(get_force)(body));

    return 1;
error:
    return 0;
}

int luab_Body_set_force(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    VPoint force = luaL_tovpoint(L, 2);
    body->_(set_force)(body, force);

    return 0;
error:
    return 0;
}

int luab_Body_get_angle(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    lua_pushnumber(L, body->_(get_angle)(body));

    return 1;
error:
    return 0;
}

int luab_Body_set_angle(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    float angle = lua_tonumber(L, 2);
    body->_(set_angle)(body, angle);

    return 0;
error:
    return 0;
}

int luab_Body_get_friction(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    lua_pushnumber(L, body->_(get_friction)(body));

    return 1;
error:
    return 0;
}

int luab_Body_set_friction(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    float friction = lua_tonumber(L, 2);
    body->_(set_friction)(body, friction);

    return 0;
error:
    return 0;
}

int luab_Body_get_elasticity(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    lua_pushnumber(L, body->_(get_elasticity)(body));

    return 1;
error:
    return 0;
}

int luab_Body_set_elasticity(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    float elasticity = lua_tonumber(L, 2);
    body->_(set_elasticity)(body, elasticity);

    return 0;
error:
    return 0;
}

int luab_Body_get_mass(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    lua_pushnumber(L, body->_(get_mass)(body));

    return 1;
error:
    return 0;
}

int luab_Body_set_mass(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    float mass = lua_tonumber(L, 2);
    body->_(set_mass)(body, mass);

    return 0;
error:
    return 0;
}

int luab_Body_get_can_rotate(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    lua_pushboolean(L, body->_(get_can_rotate)(body));

    return 1;
error:
    return 0;
}

int luab_Body_set_can_rotate(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    int can_rotate = lua_toboolean(L, 2);
    body->_(set_can_rotate)(body, can_rotate);

    return 0;
error:
    return 0;
}

int luab_Body_get_is_rogue(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    lua_pushboolean(L, body->_(get_is_rogue)(body));

    return 1;
error:
    return 0;
}

int luab_Body_set_is_rogue(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    int is_rogue = lua_toboolean(L, 2);
    body->_(set_is_rogue)(body, is_rogue);

    return 0;
error:
    return 0;
}

int luab_Body_get_is_static(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    lua_pushboolean(L, body->_(get_is_static)(body));

    return 1;
error:
    return 0;
}

int luab_Body_set_is_static(lua_State *L) {
    Body *body = luaL_tobody(L, 1);
    check(body != NULL, "Body required");
    int is_static = lua_toboolean(L, 2);
    body->_(set_is_static)(body, is_static);

    return 0;
error:
    return 0;
}

static const struct luaL_Reg luab_Body_meths[] = {
    {"__gc", luab_Body_close},
    {"add_sensor", luab_Body_add_sensor},
    {"remove_sensor", luab_Body_remove_sensor},
    {"apply_force", luab_Body_apply_force},
    {"set_hit_box", luab_Body_set_hit_box},
    {"get_entity", luab_Body_get_entity},
    {"get_pos", luab_Body_get_pos},
    {"set_pos", luab_Body_set_pos},
    {"get_velo", luab_Body_get_velo},
    {"set_velo", luab_Body_set_velo},
    {"get_force", luab_Body_get_force},
    {"set_force", luab_Body_set_force},
    {"get_angle", luab_Body_get_angle},
    {"set_angle", luab_Body_set_angle},
    {"get_friction", luab_Body_get_friction},
    {"set_friction", luab_Body_set_friction},
    {"get_elasticity", luab_Body_get_elasticity},
    {"set_elasticity", luab_Body_set_elasticity},
    {"get_mass", luab_Body_get_mass},
    {"set_mass", luab_Body_set_mass},
    {"get_can_rotate", luab_Body_get_can_rotate},
    {"set_can_rotate", luab_Body_set_can_rotate},
    {"get_is_rogue", luab_Body_get_is_rogue},
    {"set_is_rogue", luab_Body_set_is_rogue},
    {"get_is_static", luab_Body_get_is_static},
    {"set_is_static", luab_Body_set_is_static},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Body_funcs[] = {
    {"new", luab_Body_new},
    {NULL, NULL}
};

int luaopen_dabes_body(lua_State *L) {
    luaL_newmetatable(L, luab_Body_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Body_meths, 0);
    luaL_newlib(L, luab_Body_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Body_lib);

    return 1;
}
