#include "overlay_bindings.h"

const char *luab_Overlay_lib = "dab_overlay";
const char *luab_Overlay_metatable = "DaBes.overlay";

int luab_Overlay_new(lua_State *L) {
    Overlay_userdata *ud = NULL;
    Engine *engine = luaL_get_engine(L);

    char *fname = (char *)lua_tostring(L, 1);
    int px_size = (int)lua_tointeger(L, 2);

    ud = lua_newuserdata(L, sizeof(Overlay_userdata));
    check(ud != NULL, "Could not make Overlay userdata");
    ud->p = NULL;

    luaL_getmetatable(L, luab_Overlay_metatable);
    lua_setmetatable(L, -2);

    char *font_name = engine->project_path(fname);
    Overlay *overlay = Overlay_create(engine, font_name, px_size);
    free(font_name);
    luaL_register_ud(L, -1, (void **)&ud->p, overlay);
    return 1;
error:
    return 0;
}

int luab_Overlay_close(lua_State *L) {
    Overlay_userdata *ud = (Overlay_userdata *)
        luaL_checkudata(L, 1, luab_Overlay_metatable);
    if (ud->p) {
        Overlay_destroy(ud->p);
    }
    ud->p = NULL;
    return 0;
}

int luab_Overlay_draw_string(lua_State *L) {
    Overlay *overlay = luaL_tooverlay(L, 1);
    check(overlay != NULL, "Overlay required");

    const char *str = lua_tostring(L, 2);
    VVector4 color = luaL_tovvector4(L, 3);
    VPoint origin = luaL_tovpoint(L, 4);
    const char *align = lua_tostring(L, 5);

    GfxTextAlign text_align = GfxTextAlignLeft;
    if (streq(align, "center")) text_align = GfxTextAlignCenter;
    else if (streq(align, "right")) text_align = GfxTextAlignRight;

    VVector4 shadow_color = VVector4Zero;
    if (!lua_isnoneornil(L, 6)) shadow_color = luaL_tovvector4(L, 6);
  
    VPoint shadow_offset = VPointZero;
    if (!lua_isnoneornil(L, 7)) shadow_offset = luaL_tovpoint(L, 7);

    Engine *engine = luaL_get_engine(L);

    GfxShader *txshader = Graphics_get_shader(engine->graphics, "text");
    Graphics_use_shader(engine->graphics, txshader);
    Graphics_draw_string(engine->graphics, (char *)str, overlay->font,
            color.raw, origin, text_align, shadow_color.raw, shadow_offset);

    return 1;
error:
    return 0;
}

static const struct luaL_Reg luab_Overlay_meths[] = {
    {"__gc", luab_Overlay_close},
    {"draw_string", luab_Overlay_draw_string},
    {NULL, NULL}
};

static const struct luaL_Reg luab_Overlay_funcs[] = {
    {"new", luab_Overlay_new},
    {NULL, NULL}
};

int luaopen_dabes_overlay(lua_State *L) {
    luaL_newmetatable(L, luab_Overlay_metatable);
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, luab_Overlay_meths, 0);
    luaL_newlib(L, luab_Overlay_funcs);
    lua_pushvalue(L, -1);
    lua_setglobal(L, luab_Overlay_lib);

    return 1;
}
