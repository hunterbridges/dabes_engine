#include "overlay_bindings.h"
#include "../graphics/graphics.h"
#include "../graphics/sprite_bindings.h"
#include "../entities/entity_bindings.h"
#include "../math/vrect.h"

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

    Overlay *overlay = NULL;
    if (fname) {
        char *font_name = engine->project_path(fname);
        overlay = Overlay_create(engine, font_name, px_size);
        free(font_name);
    } else {
        overlay = Overlay_create(engine, NULL, 0);
    }
    luaL_register_ud(L, -1, (void **)&ud->p, overlay);
    return 1;
error:
    return 0;
}

int luab_Overlay_close(lua_State *L) {
    Overlay_userdata *ud = (Overlay_userdata *)
        luaL_testudata(L, 1, luab_Overlay_metatable);
    if (ud->p) {
        Overlay_destroy(ud->p);
    }
    ud->p = NULL;
    return 0;
}

typedef struct OverlayDrawStringCtx {
    Overlay *overlay;
    VVector4 color;
    VPoint origin;
    GfxTextAlign text_align;
    VVector4 shadow_color;
    VPoint shadow_offset;
    VMatrix projection;
    VMatrix modelview;
    char *str;
} OverlayDrawStringCtx;

void overlay_draw_string(DrawEvent *event, Graphics *graphics) {
    OverlayDrawStringCtx *ctx = event->context;
    graphics->projection_matrix = ctx->projection;
    Graphics_uniformMatrix4fv(graphics, UNIFORM_TEXT_MODELVIEW_MATRIX,
                              ctx->modelview.gl, GL_FALSE);
    Graphics_draw_string(graphics, ctx->str, ctx->overlay->font,
            ctx->color.raw, ctx->origin, ctx->text_align, ctx->shadow_color.raw,
            ctx->shadow_offset, event->z);
}

int luab_Overlay_draw_string(lua_State *L) {
    Overlay *overlay = luaL_tooverlay(L, 1);
    check(overlay != NULL, "Overlay required");

    const char *str = lua_tostring(L, 2);
    VVector4 color = luaL_tovvector4(L, 3);
    VPoint origin = luaL_tovpoint(L, 4);
    const char *align = lua_tostring(L, 5);
    lua_Number z = lua_tonumber(L, 8);

    origin = VPoint_add(origin, overlay->track_entity_offset);

    GfxTextAlign text_align = GfxTextAlignLeft;
    if (streq(align, "center")) text_align = GfxTextAlignCenter;
    else if (streq(align, "right")) text_align = GfxTextAlignRight;

    VVector4 shadow_color = VVector4Zero;
    if (!lua_isnoneornil(L, 6)) shadow_color = luaL_tovvector4(L, 6);

    VPoint shadow_offset = VPointZero;
    if (!lua_isnoneornil(L, 7)) shadow_offset = luaL_tovpoint(L, 7);

    Engine *engine = luaL_get_engine(L);

    GfxShader *txshader = Graphics_get_shader(engine->graphics, "text");

    OverlayDrawStringCtx *ctx = calloc(1, sizeof(OverlayDrawStringCtx));
    ctx->overlay = overlay;
    ctx->color = color;
    ctx->origin = origin;
    ctx->text_align = text_align;
    ctx->shadow_color = shadow_color;
    ctx->shadow_offset = shadow_offset;
    ctx->projection = engine->graphics->projection_matrix;
    ctx->modelview = engine->graphics->modelview_matrix;
    ctx->str = calloc(1, strlen(str) + 1);
    strcpy(ctx->str, str);

    DrawEvent *event = DrawEvent_create(DRAW_EVENT_OVERLAY_STRING, z, txshader);
    event->context = ctx;
    event->func = overlay_draw_string;
    Graphics_enqueue_draw_event(engine->graphics, event);

    return 1;
error:
    return 0;
}

typedef struct OverlayDrawSpriteCtx {
    Overlay *overlay;
    Sprite *sprite;
    VRect rect;
    VVector4 color;
    VMatrix projection;
    float rotation;
} OverlayDrawSpriteCtx;

void overlay_draw_sprite(DrawEvent *event, Graphics *graphics) {
    OverlayDrawSpriteCtx *ctx = event->context;

    Graphics_uniformMatrix4fv(graphics,
                              UNIFORM_DECAL_PROJECTION_MATRIX,
                              ctx->projection.gl, GL_FALSE);
    Graphics_draw_sprite(graphics, ctx->sprite, NULL,
            ctx->rect, ctx->color.raw, ctx->rotation, ctx->overlay->alpha,
            event->z);
}

int luab_Overlay_draw_sprite(lua_State *L) {
    Overlay *overlay = luaL_tooverlay(L, 1);
    check(overlay != NULL, "Overlay required");

    Sprite *sprite = luaL_tosprite(L, 2);
    VVector4 color = luaL_tovvector4(L, 3);
    VPoint center = luaL_tovpoint(L, 4);
    float rotation = lua_tonumber(L, 5);
    lua_Number z = lua_tonumber(L, 7);

    VPoint scale = {1, 1};
    if (lua_type(L, 6) == LUA_TTABLE) {
      scale = luaL_tovpoint(L, 6);
    } else if (lua_type(L, 6) == LUA_TNUMBER) {
      float s = lua_tonumber(L, 6);
      scale.x = s;
      scale.y = s;
    }

    Engine *engine = luaL_get_engine(L);

    VRect rect = VRect_from_xywh(center.x - scale.x * sprite->cell_size.w / 2.0
                                     + overlay->track_entity_offset.x,
                                 center.y - scale.y * sprite->cell_size.h / 2.0
                                     + overlay->track_entity_offset.y,
                                 sprite->cell_size.w * scale.x,
                                 sprite->cell_size.h * scale.y);
    GfxShader *dshader = Graphics_get_shader(engine->graphics, "decal");

    OverlayDrawSpriteCtx *ctx = calloc(1, sizeof(OverlayDrawSpriteCtx));
    ctx->overlay = overlay;
    ctx->sprite = sprite;
    ctx->projection = engine->graphics->projection_matrix;
    ctx->color = color;
    ctx->rotation = rotation;
    ctx->rect = rect;

    DrawEvent *event = DrawEvent_create(DRAW_EVENT_OVERLAY_SPRITE, z,
                                        dshader);
    event->context = ctx;
    event->func = overlay_draw_sprite;
    Graphics_enqueue_draw_event(engine->graphics, event);

    return 1;
error:
    return 0;
}

int luab_Overlay_get_scene(lua_State *L) {
    Overlay *overlay = luaL_tooverlay(L, 1);
    check(overlay != NULL, "Overlay required");
    if (overlay->scene == NULL) {
        lua_pushnil(L);
        return 1;
    }
    luaL_lookup_instance(L, overlay->scene);

    return 1;
error:
    return 0;
}

int luab_Overlay_add_sprite(lua_State *L) {
    Overlay *overlay = luaL_tooverlay(L, 1);
    check(overlay != NULL, "Overlay required");
    lua_getfield(L, 2, "real");
    Sprite *sprite = luaL_tosprite(L, -1);
    Overlay_add_sprite(overlay, sprite);

    return 0;
error:
    return 0;
}

int luab_Overlay_get_track_entity(lua_State *L) {
    Overlay *overlay = luaL_tooverlay(L, 1);
    check(overlay != NULL, "Overlay required");
    int rc = luaL_lookup_instance(L, overlay->track_entity);
    if (rc == 0) {
        lua_pushnil(L);
    }

    return 1;
error:
    return 0;
}

int luab_Overlay_set_track_entity(lua_State *L) {
    Overlay *overlay = luaL_tooverlay(L, 1);
    check(overlay != NULL, "Overlay required");

    Entity *entity = luaL_toentity(L, -1);
    overlay->track_entity = entity;

    return 0;
error:
    return 0;
}

Scripting_num_getter(Overlay, track_entity_edge);
Scripting_num_setter(Overlay, track_entity_edge);

static const struct luaL_Reg luab_Overlay_meths[] = {
    {"__gc", luab_Overlay_close},
    {"draw_string", luab_Overlay_draw_string},
    {"draw_sprite", luab_Overlay_draw_sprite},
    {"get_scene", luab_Overlay_get_scene},
    {"add_sprite", luab_Overlay_add_sprite},
    {"get_track_entity", luab_Overlay_get_track_entity},
    {"set_track_entity", luab_Overlay_set_track_entity},
    {"get_track_entity_edge", luab_Overlay_get_track_entity_edge},
    {"set_track_entity_edge", luab_Overlay_set_track_entity_edge},
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
