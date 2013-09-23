#include "recorder_bindings.h"
#include "../core/engine.h"
#include "../entities/entity_bindings.h"

const char *luab_Recorder_lib = "dab_recorder";
const char *luab_Recorder_metatable = "DaBes.recorder";

int luab_Recorder_new(lua_State *L) {
    Recorder_userdata *ud = NULL;

    ud = lua_newuserdata(L, sizeof(Recorder_userdata));
    check(ud != NULL, "Could not make Recorder userdata");
    ud->p = NULL;

    luaL_getmetatable(L, luab_Recorder_metatable);
    lua_setmetatable(L, -2);

    lua_Integer preroll = lua_tointeger(L, 1);
    Recorder *recorder = Recorder_create((int)preroll, FPS);
    luaL_register_ud(L, -1, (void **)&ud->p, recorder);

    return 1;
error:
    return 0;
}

Scripting_destroy_closer(Recorder);

Scripting_num_getter(Recorder, state);

int luab_Recorder_play(lua_State *L) {
    Recorder *recorder = luaL_torecorder(L, 1);
    check(recorder != NULL, "Recorder required");

    Recorder_set_state(recorder, RecorderStatePlaying);

    return 0;
error:
    return 0;
}

int luab_Recorder_pause(lua_State *L) {
    Recorder *recorder = luaL_torecorder(L, 1);
    check(recorder != NULL, "Recorder required");

    Recorder_set_state(recorder, RecorderStateIdle);

    return 0;
error:
    return 0;
}

int luab_Recorder_rewind(lua_State *L) {
    Recorder *recorder = luaL_torecorder(L, 1);
    check(recorder != NULL, "Recorder required");

    recorder->_(rewind)(recorder);

    return 0;
error:
    return 0;
}

int luab_Recorder_record(lua_State *L) {
    Recorder *recorder = luaL_torecorder(L, 1);
    check(recorder != NULL, "Recorder required");

    Recorder_set_state(recorder, RecorderStateRecording);

    return 0;
error:
    return 0;
}

int luab_Recorder_get_entity(lua_State *L) {
    Recorder *recorder = luaL_torecorder(L, 1);
    check(recorder != NULL, "Recorder required");
    if (recorder->entity == NULL) {
        lua_pushnil(L);
        return 1;
    }

    luaL_lookup_instance(L, recorder->entity);
    return 1;
error:
    return 0;
}

int luab_Recorder_set_entity(lua_State *L) {
    Recorder *recorder = luaL_torecorder(L, 1);
    check(recorder != NULL, "Recorder required");

    lua_getfield(L, -1, "real");
    Entity *entity = luaL_toentity(L, -1);

    recorder->entity = entity;

    lua_pop(L, 3);
error:
    return 0;
}

int luab_Recorder_clear(lua_State *L) {
    Recorder *recorder = luaL_torecorder(L, 1);
    check(recorder != NULL, "Recorder required");

    recorder->_(clear_frames)(recorder);

    return 0;
error:
    return 0;
}

static const struct luaL_Reg luab_Recorder_meths[] = {
    {"__gc", luab_Recorder_close},
    {"play", luab_Recorder_play},
    {"pause", luab_Recorder_pause},
    {"rewind", luab_Recorder_rewind},
    {"record", luab_Recorder_record},
    {"clear", luab_Recorder_clear},
    {"get_state", luab_Recorder_get_state},
    {"get_entity", luab_Recorder_get_entity},
    {"set_entity", luab_Recorder_set_entity}
};

static const struct luaL_Reg luab_Recorder_funcs[] = {
    {"new", luab_Recorder_new},
    {NULL, NULL}
};

int luaopen_dabes_recorder(lua_State *L) {
    luaL_newmetatable(L, luab_Recorder_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Recorder_meths, 0);
    luaL_newlib(L, luab_Recorder_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Recorder_lib);

    return 1;
}
