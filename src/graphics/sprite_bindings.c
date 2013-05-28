#include "sprite_bindings.h"

const char *luab_SpriteAnimation_lib = "dab_spriteanimation";
const char *luab_SpriteAnimation_metatable = "DaBes.spriteanimation";

Scripting_num_getter(SpriteAnimation, current_index);
Scripting_num_setter(SpriteAnimation, current_index);

static const struct luaL_Reg luab_SpriteAnimation_meths[] = {
    {"__gc", luab_SpriteAnimation_close},
    {"get_current_index", luab_SpriteAnimation_get_current_index},
    {"set_current_index", luab_SpriteAnimation_set_current_index},
    {"get_fps", luab_SpriteAnimation_get_fps},
    {"set_fps", luab_SpriteAnimation_set_fps},
    {NULL, NULL}
};

static const struct luaL_Reg luab_SpriteAnimation_funcs[] = {
    {"new", luab_SpriteAnimation_new},
    {NULL, NULL}
};

////////////////////////////////////////////////////////////////////////////////

const char *luab_Sprite_lib = "dab_sprite";
const char *luab_Sprite_metatable = "DaBes.sprite";

static const struct luaL_Reg luab_Sprite_meths[] = {
    {"__gc", luab_Sprite_close},
    {"add_animation", luab_Sprite_add_animation},
    {"use_animation", luab_Sprite_use_animation},
    {"get_current_animation", luab_Sprite_get_current_animation},
    {"get_direction", luab_Sprite_get_direction},
    {"set_direction", luab_Sprite_set_direction},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Sprite_funcs[] = {
    {"new", luab_Sprite_new},
    {NULL, NULL}
};

int luaopen_dabes_parallax(lua_State *L) {
    luaL_newmetatable(L, luab_SpriteAnimation_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_SpriteAnimation_meths, 0);
    luaL_newlib(L, luab_SpriteAnimation_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_SpriteAnimation_lib);

    luaL_newmetatable(L, luab_Sprite_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Sprite_meths, 0);
    luaL_newlib(L, luab_Sprite_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Sprite_lib);

    return 1;
}
