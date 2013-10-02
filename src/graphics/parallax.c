#include "parallax.h"
#include "draw_buffer.h"
#include "../util.h"
#include "../scenes/scene.h"

ParallaxLayer *ParallaxLayer_create(GfxTexture *tex) {
    ParallaxLayer *layer = calloc(1, sizeof(ParallaxLayer));
    check(layer != NULL, "Could not create layer");

    layer->scale = 1;
    layer->texture = tex;
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
Parallax *Parallax_create() {
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

void Parallax_render(Parallax *parallax, Graphics *graphics) {
    //double bgScale = (parallax->camera->scale + 2) / 2;
    GfxShader *dshader = Graphics_get_shader(graphics, "decal");
    GfxShader *pshader = Graphics_get_shader(graphics, "parallax");

    Graphics_use_shader(graphics, dshader);
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
    Graphics_uniformMatrix4fv(graphics,
                              UNIFORM_DECAL_PROJECTION_MATRIX,
                              graphics->projection_matrix.gl,
                              GL_FALSE);
  
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

    // render sea color
    VRect sea_rect = VRect_from_xywh(
            0 - (hyp - screen_size.w) / 2.0,
            0 - (hyp - screen_size.h) / 2.0,
            hyp, hyp);
    Graphics_draw_rect(graphics, dshader->draw_buffer, sea_rect, parallax->sea_color.raw,
        NULL, VPointZero, GfxSizeZero, 0, 0, 1);

    // render sky color
    VRect sky_rect = VRect_from_xywh(
            0 - (hyp - screen_size.w) / 2.0,
            0 - (hyp - screen_size.h) / 2.0,
            hyp, hyp / 2.0 - y_wiggle);
    Graphics_draw_rect(graphics, dshader->draw_buffer, sky_rect, parallax->sky_color.raw,
        NULL, VPointZero, GfxSizeZero, 0, 0, 1);
    DrawBuffer_draw(dshader->draw_buffer, graphics);
    DrawBuffer_empty(dshader->draw_buffer);

    Graphics_use_shader(graphics, pshader);
    Graphics_reset_modelview_matrix(graphics);

    Graphics_uniformMatrix4fv(graphics,
                              UNIFORM_PARALLAX_PROJECTION_MATRIX,
                              graphics->projection_matrix.gl,
                              GL_FALSE);
    Graphics_uniformMatrix4fv(graphics,
                              UNIFORM_PARALLAX_MODELVIEW_MATRIX,
                              graphics->modelview_matrix.gl,
                              GL_FALSE);
    Graphics_uniform2f(graphics,
                       UNIFORM_PARALLAX_CAMERA_POS,
                       cam_pos.x,
                       cam_pos.y);

    VPoint stretch = {
        hyp / screen_size.w,
        hyp / screen_size.h
    };
  
    VVector4 texpos[4] = {
      {.raw = {0,0,0,0}},
      {.raw = {stretch.x,0,0,0}},
      {.raw = {0,1.0,0,0}},
      {.raw = {stretch.x,1.0,0,0}}
    };

    int i = 0;
    for (i = 0; i < DArray_count(parallax->layers); i++) {
        ParallaxLayer *layer = DArray_get(parallax->layers, i);
        assert(layer->texture != NULL);
        GfxTexture *texture = layer->texture;

        double sx = (layer->scale * bg_scale);
        double final_scale = sx;
        // Some other wacko easing functions...
        // double final_scale = 1 + (sx - 1) * (pow(layer->p_factor + 1, 2) - 1);
        // double final_scale = 1 + (sx - 1) * layer->p_factor;
        double layer_wiggle = layer->y_wiggle * bg_scale * wiggle_factor;

        VRect rect = VRect_from_xywh(
                0 - (hyp - parallax->camera->screen_size.w) / 2.0,
                screen_center.y + layer->offset.y * final_scale
                    - (y_wiggle + layer_wiggle),
                hyp,
                texture->size.h * final_scale
        );
        rect = VRect_round_out(rect);

        double repeat_width = layer->texture->size.w * final_scale /
            parallax->camera->screen_size.w;
        Graphics_uniform2f(graphics,
                           UNIFORM_PARALLAX_REPEAT_SIZE,
                           repeat_width, 1.0);
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

        int j = 0;
        for (j = 0; j < 4; j++) {
          float adjust = 0;
          texpos[j].raw[0] += adjust;
        }
      
        VVector4 vertices[8] = {
          // Vertex
          {.raw = {rect.tl.x, rect.tl.y, 0, 1}},
          {.raw = {rect.tr.x, rect.tr.y, 0, 1}},
          {.raw = {rect.bl.x, rect.bl.y, 0, 1}},
          {.raw = {rect.br.x, rect.br.y, 0, 1}},

          // Texture
          texpos[0], texpos[1], texpos[2], texpos[3]
        };
        glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(VVector4), vertices,
                GL_STATIC_DRAW);
      
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
      
        Graphics_uniform1i(graphics,
                           UNIFORM_PARALLAX_TEXTURE,
                           0);
        glBindTexture(GL_TEXTURE_2D, texture->gl_tex);
        
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}
