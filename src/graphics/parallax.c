#include "parallax.h"
#include "draw_buffer.h"
#include "../util.h"
#include "../scenes/scene.h"

ParallaxLayer *ParallaxLayer_create(GfxTexture *tex) {
    ParallaxLayer *layer = calloc(1, sizeof(ParallaxLayer));
    check(layer != NULL, "Could not create layer");

    layer->scale = 1;
    layer->texture = tex;
    layer->z = -180.f;
    memcpy(&layer->texture_size, &layer->texture->size, sizeof(GfxSize));

    ParallaxLayer_p_cascade(layer, 1, 1);

    return layer;
error:
    return NULL;
}

void ParallaxLayer_p_cascade(ParallaxLayer *layer, double top, double bot) {
    check(layer != NULL, "No ParallaxLayer to cascade");

    layer->p_factor = MAX(top, bot);

    layer->cascade_top = top / layer->p_factor;
    layer->cascade_bottom = bot / layer->p_factor;

    return;
error:
    return;
}

////////////////////////////////////////////////////////////////////////////////
Parallax *Parallax_create(Engine *UNUSED(engine)) {
    Parallax *parallax = malloc(sizeof(Parallax));
    check(parallax != NULL, "Could not create parallax");

    parallax->layers = DArray_create(sizeof(ParallaxLayer *), 8);
    parallax->camera = NULL;
    GfxSize level = {1, 1};
    parallax->level_size = level;

    parallax->sky_color.r = 0.5;
    parallax->sky_color.g = 0.5;
    parallax->sky_color.b = 1.0;
    parallax->sky_color.a = 1.0;

    parallax->sea_color.r = 0.0;
    parallax->sea_color.g = 0.5;
    parallax->sea_color.b = 0.0;
    parallax->sea_color.a = 1.0;

    parallax->y_wiggle = 0.0;
    parallax->sea_level = 1.0;

    parallax->bg_z = -200.f;

    return parallax;
error:
    if (parallax) free(parallax);
    return NULL;
}

void Parallax_destroy(Parallax *parallax) {
    check(parallax != NULL, "No parallax to destroy");

    DArray_destroy(parallax->layers);
    free(parallax);

    return;
error:
    return;
}

int Parallax_add_layer(Parallax *parallax, ParallaxLayer *layer) {
    check(parallax != NULL, "No parallax to add to");
    check(layer != NULL, "No parallax layer to add");

    DArray_push(parallax->layers, layer);

    return 1;
error:
    return 0;
}

typedef struct ParallaxBgDrawCtx {
    Parallax *parallax;
    VMatrix projection;
    GfxSize screen_size;
    float hyp;
    double y_wiggle;
} ParallaxBgDrawCtx;

void parallax_draw_bg(DrawEvent *event, Graphics *graphics) {
    ParallaxBgDrawCtx *ctx = event->context;

    Parallax *parallax = ctx->parallax;
    GfxSize screen_size = ctx->screen_size;
    float hyp = ctx->hyp;
    double y_wiggle = ctx->y_wiggle;

    Graphics_uniformMatrix4fv(graphics,
                              UNIFORM_DECAL_PROJECTION_MATRIX,
                              ctx->projection.gl,
                              GL_FALSE);

    // render sea color
    VRect sea_rect = VRect_from_xywh(
            0 - (hyp - screen_size.w) / 2.0,
            0 - (hyp - screen_size.h) / 2.0,
            hyp, hyp);
    Graphics_draw_rect(graphics, NULL, sea_rect, parallax->sea_color.raw,
        NULL, VPointZero, GfxSizeZero, 0, 1, parallax->bg_z);

    // render sky color
    VRect sky_rect = VRect_from_xywh(
            0 - (hyp - screen_size.w) / 2.0,
            0 - (hyp - screen_size.h) / 2.0,
            hyp, hyp / 2.0 - y_wiggle);
    Graphics_draw_rect(graphics, event->shader->draw_buffer, sky_rect, parallax->sky_color.raw,
        NULL, VPointZero, GfxSizeZero, 0, 1, parallax->bg_z + 10);
    DrawBuffer_draw(event->shader->draw_buffer, graphics);
    DrawBuffer_empty(event->shader->draw_buffer);
}

typedef struct ParallaxLayerDrawCtx {
    Parallax *parallax;
    VMatrix projection;
    VPoint cam_pos;
    VPoint stretch;
    VPoint screen_center;
    double y_wiggle;
    double bg_scale;
    double wiggle_factor;
    float hyp;

    ParallaxLayer *layer;
} ParallaxLayerDrawCtx;

void parallax_layer_draw(DrawEvent *event, Graphics *graphics) {
    ParallaxLayerDrawCtx *ctx = event->context;

    Parallax *parallax = ctx->parallax;
    ParallaxLayer *layer = ctx->layer;
    double bg_scale = ctx->bg_scale;
    double wiggle_factor = ctx->wiggle_factor;
    double y_wiggle = ctx->y_wiggle;
    VPoint screen_center = ctx->screen_center;
    float hyp = ctx->hyp;

    Graphics_uniformMatrix4fv(graphics,
                              UNIFORM_PARALLAX_PROJECTION_MATRIX,
                              ctx->projection.gl,
                              GL_FALSE);
    Graphics_uniform2f(graphics,
                       UNIFORM_PARALLAX_CAMERA_POS,
                       ctx->cam_pos.x,
                       ctx->cam_pos.y);

    Graphics_uniform2f(graphics,
                       UNIFORM_PARALLAX_STRETCH,
                       ctx->stretch.x,
                       ctx->stretch.y);
    Graphics_reset_modelview_matrix(graphics);

    GfxTexture *texture = layer->texture;

    double sx = (layer->scale * bg_scale);
    double final_scale = sx;
    // Some other wacko easing functions...
    // double final_scale = 1 + (sx - 1) * (pow(layer->p_factor + 1, 2) - 1);
    // double final_scale = 1 + (sx - 1) * layer->p_factor;
    double layer_wiggle = layer->y_wiggle * bg_scale * wiggle_factor;

    Graphics_reset_modelview_matrix(graphics);
    Graphics_translate_modelview_matrix(graphics, 0 - (hyp - parallax->camera->screen_size.w),
                                        screen_center.y + layer->offset.y * final_scale
                                          - (y_wiggle + layer_wiggle),
                                        event->z);
    Graphics_scale_modelview_matrix(graphics, hyp, texture->size.h * final_scale, 1);
    Graphics_uniformMatrix4fv(graphics,
                              UNIFORM_PARALLAX_MODELVIEW_MATRIX,
                              graphics->modelview_matrix.gl,
                              GL_FALSE);

    double repeat_width = layer->texture->size.w * final_scale /
        parallax->camera->screen_size.w;

    // NOTE: I am storing the reciprocal width in the y slot here.
    // If this were to ever be modified to do Y parallax, then the Y would
    // be used and the reciprocals would need to be moved to a new uniform.
    Graphics_uniform2f(graphics,
                       UNIFORM_PARALLAX_REPEAT_SIZE,
                       repeat_width, 1.0 / repeat_width);
    Graphics_uniform1f(graphics,
                       UNIFORM_PARALLAX_TEX_SCALE,
                       final_scale);

    VPoint pot_scale = {
        texture->size.w / texture->pot_size.w,
        texture->size.h / texture->pot_size.h
    };
    Graphics_uniform2f(graphics,
                       UNIFORM_PARALLAX_TEX_PORTION,
                       pot_scale.x, pot_scale.y);

    double repeats =
        parallax->level_size.w / (texture->size.w * final_scale);
    Graphics_uniform1f(graphics,
                       UNIFORM_PARALLAX_REPEATS,
                       repeats);

    // Seems odd that I have to flip the sign here...
    float x_shift = -layer->offset.x / layer->texture->size.w;

    Graphics_uniform1f(graphics,
                       UNIFORM_PARALLAX_X_SHIFT,
                       x_shift);

    Graphics_uniform1f(graphics,
                       UNIFORM_PARALLAX_FACTOR,
                       layer->p_factor);

    Graphics_uniform2f(graphics,
                       UNIFORM_PARALLAX_ORIG_PIXEL,
                       1 / layer->texture->size.w,
                       1 / layer->texture->size.h);
    Graphics_uniform1f(graphics,
                       UNIFORM_PARALLAX_CASCADE_TOP,
                       layer->cascade_top);
    Graphics_uniform1f(graphics,
                       UNIFORM_PARALLAX_CASCADE_BOTTOM,
                       layer->cascade_bottom);

    glBindTexture(GL_TEXTURE_2D, texture->gl_tex);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void Parallax_render(Parallax *parallax, Graphics *graphics) {
    GfxShader *dshader = Graphics_get_shader(graphics, "decal");
    GfxShader *pshader = Graphics_get_shader(graphics, "parallax");

    // Prep
    double bg_scale = parallax->camera->scale;
    VPoint screen_center = {
      .x = parallax->camera->screen_size.w / 2.0,
      .y = parallax->camera->screen_size.h / 2.0
    };
    Camera bg_camera = {
        .focal = screen_center,
        .screen_size = parallax->camera->screen_size,
        .scale = 1.0,
        .rotation_radians = parallax->camera->rotation_radians
    };
    Camera_snap_tracking(&bg_camera);
    Graphics_project_camera(graphics, &bg_camera);

    float pixel_x =
        parallax->camera->tracking.focal.x
        / parallax->camera->screen_size.w;
    VPoint cam_pos = {
        pixel_x,
        floorf(parallax->camera->tracking.focal.y) / parallax->level_size.h
    };

    GfxSize screen_size = parallax->camera->screen_size;
    float hyp = sqrtf(powf(parallax->camera->screen_size.w, 2) +
            powf(parallax->camera->screen_size.h, 2));

    double wiggle_factor = cam_pos.y - parallax->sea_level; // -0.5 to 0.5
    double y_wiggle = parallax->y_wiggle * bg_scale * wiggle_factor;

    // Draw BG event
    ParallaxBgDrawCtx *bg_ctx = calloc(1, sizeof(ParallaxBgDrawCtx));
    bg_ctx->hyp = hyp;
    bg_ctx->y_wiggle = y_wiggle;
    bg_ctx->projection = graphics->projection_matrix;
    bg_ctx->screen_size = screen_size;
    bg_ctx->parallax = parallax;

    DrawEvent *bg_event = DrawEvent_create(DRAW_EVENT_PARALLAX_BG,
                                           parallax->bg_z, dshader);
    bg_event->context = bg_ctx;
    bg_event->func = parallax_draw_bg;
    if (parallax->sky_color.a >= 1.0 && parallax->sea_color.a >= 1.0) {
      bg_event->opaque = 1;
    }
    Graphics_enqueue_draw_event(graphics, bg_event);

    VPoint stretch = {
        hyp / screen_size.w,
        hyp / screen_size.h
    };

    ParallaxLayerDrawCtx ctx_tpl = {
        .parallax = parallax,
        .cam_pos = cam_pos,
        .projection = graphics->projection_matrix,
        .stretch = stretch,
        .bg_scale = bg_scale,
        .hyp = hyp,
        .wiggle_factor = wiggle_factor,
        .y_wiggle = y_wiggle,
        .screen_center = screen_center
    };

    int i = 0;
    for (i = 0; i < DArray_count(parallax->layers); i++) {
        ParallaxLayer *layer = DArray_get(parallax->layers, i);
        assert(layer->texture != NULL);

        ParallaxLayerDrawCtx *ctx = calloc(1, sizeof(ParallaxLayerDrawCtx));
        *ctx = ctx_tpl;
        ctx->layer = layer;

        DrawEvent *layer_event = DrawEvent_create(DRAW_EVENT_PARALLAX_LAYER,
                                                  layer->z, pshader);
        layer_event->context = ctx;
        layer_event->func = parallax_layer_draw;
        Graphics_enqueue_draw_event(graphics, layer_event);
    }
}
