#include <lcthw/bstrlib.h>
#include "sprite_bindings.h"

const char *luab_SpriteAnimation_lib = "dab_spriteanimation";
const char *luab_SpriteAnimation_metatable = "DaBes.spriteanimation";

Scripting_num_getter(SpriteAnimation, current_index);
Scripting_num_setter(SpriteAnimation, current_index);

int luab_SpriteAnimation_new(lua_State *L) {
    SpriteAnimation_userdata *ud = NULL;

    int num_frames = lua_gettop(L);
    int frames[num_frames];

    ud = lua_newuserdata(L, sizeof(SpriteAnimation_userdata));
    check(ud != NULL, "Could not make SpriteAnimation userdata");

    luaL_getmetatable(L, luab_SpriteAnimation_metatable);
    lua_setmetatable(L, -2);

    int i = 0;
    for (i = 0; i < num_frames; i++) {
        frames[i] = (int)lua_tointeger(L, 1);
        lua_remove(L, 1);
    }

    SpriteAnimation *anim = SpriteAnimation_create(num_frames, frames);
    luaL_register_ud(L, -1, (void **)&ud->p, anim);
    return 1;
error:
    return 0;
}

Scripting_destroy_closer(SpriteAnimation);

int luab_SpriteAnimation_get_fps(lua_State *L) {
    SpriteAnimation *animation = luaL_tospriteanimation(L, 1);
    check(animation != NULL, "SpriteAnimation required");
    lua_pushinteger(L, animation->stepper->steps_per_second);
    return 1;
error:
    return 0;
}

int luab_SpriteAnimation_set_fps(lua_State *L) {
    SpriteAnimation *animation = luaL_tospriteanimation(L, 1);
    check(animation != NULL, "SpriteAnimation required");
    Stepper_set_steps_per_second(animation->stepper,
            lua_tointeger(L, 2));
    return 0;
error:
    return 0;
}

Scripting_bool_getter(SpriteAnimation, repeats);
Scripting_bool_setter(SpriteAnimation, repeats);

static const struct luaL_Reg luab_SpriteAnimation_meths[] = {
    {"__gc", luab_SpriteAnimation_close},
    {"get_current_index", luab_SpriteAnimation_get_current_index},
    {"set_current_index", luab_SpriteAnimation_set_current_index},
    {"get_fps", luab_SpriteAnimation_get_fps},
    {"set_fps", luab_SpriteAnimation_set_fps},
    {"get_repeats", luab_SpriteAnimation_get_repeats},
    {"set_repeats", luab_SpriteAnimation_set_repeats},
    {NULL, NULL}
};

static const struct luaL_Reg luab_SpriteAnimation_funcs[] = {
    {"new", luab_SpriteAnimation_new},
    {NULL, NULL}
};

////////////////////////////////////////////////////////////////////////////////

const char *luab_Sprite_lib = "dab_sprite";
const char *luab_Sprite_metatable = "DaBes.sprite";

int luab_Sprite_new(lua_State *L) {
    Sprite_userdata *ud = NULL;
    Engine *engine = luaL_get_engine(L);

    ud = lua_newuserdata(L, sizeof(Sprite_userdata));
    check(ud != NULL, "Could not make Sprite userdata");

    luaL_getmetatable(L, luab_Sprite_metatable);
    lua_setmetatable(L, -2);

    const char *texname = lua_tostring(L, 1);
    GfxTexture *tex = Graphics_texture_from_image(engine->graphics,
            (char *)texname);
    check(tex != NULL, "Couldn't load image %s", texname);

    int padding = lua_tonumber(L, 3);
    VPoint cellp = luaL_tovpoint(L, 2);
    GfxSize cell_size = {cellp.x, cellp.y};
    Sprite *sprite = Sprite_create(tex, cell_size, padding);

    luaL_register_ud(L, -1, (void **)&ud->p, sprite);
    return 1;
error:
    return 0;
}

Scripting_destroy_closer(Sprite);

int luab_Sprite_add_animation(lua_State *L) {
    Sprite *sprite = luaL_tosprite(L, 1);
    check(sprite != NULL, "Sprite required");
    lua_getfield(L, 2, "real");
    SpriteAnimation *anim = luaL_tospriteanimation(L, -1);
    const char *name = lua_tostring(L, 3);
    Sprite_add_animation(sprite, anim, name);

    return 0;
error:
    return 0;
}

int luab_Sprite_get_animation(lua_State *L) {
    const char *name = lua_tostring(L, 2);
    Sprite *sprite = luaL_tosprite(L, 1);
    check(sprite != NULL, "Sprite required");
    bstring bname = bfromcstr(name);
    SpriteAnimation *animation = Hashmap_get(sprite->animations, (void *)bname);
    bdestroy(bname);
    if (animation == NULL) {
        lua_pushnil(L);
        return 1;
    }

    luaL_lookup_instance(L, animation);

    return 1;
error:
    return 0;
}

int luab_Sprite_use_animation(lua_State *L) {
    Sprite *sprite = luaL_tosprite(L, 1);
    check(sprite != NULL, "Sprite required");
    const char *name = lua_tostring(L, 2);
    Sprite_use_animation(sprite, name);

    return 0;
error:
    return 0;
}

Scripting_num_getter(Sprite, direction);
Scripting_num_setter(Sprite, direction);

int luab_Sprite_get_current_animation(lua_State *L) {
    Sprite *sprite = luaL_tosprite(L, 1);
    check(sprite != NULL, "Sprite required");
    if (sprite->current_animation == NULL) {
        lua_pushnil(L);
        return 1;
    }
    luaL_lookup_instance(L, sprite->current_animation);

    return 1;
error:
    return 0;
}

static const struct luaL_Reg luab_Sprite_meths[] = {
    {"__gc", luab_Sprite_close},
    {"add_animation", luab_Sprite_add_animation},
    {"get_animation", luab_Sprite_get_animation},
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

int luaopen_dabes_sprite(lua_State *L) {
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
