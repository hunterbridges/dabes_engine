#include "telemetry_splash_scene.h"
#include "../core/easer.h"
#include "../core/engine.h"
#include "../audio/audio.h"
#include "../audio/sfx.h"
#include "../entities/body.h"
#include "../entities/body_bindings.h"
#include "../entities/sensor.h"
#include "../graphics/draw_buffer.h"
#include "../layers/overlay.h"
#include "scene.h"

static VPoint AXIS_CENTERING = {-23, 11};

typedef struct TelemetrySplashSceneCtx {
    VVector4 line_color;
    VVector4 text_color;
    float extend;
    Overlay *text_overlay;
    int sfx;
    int finished;
} TelemetrySplashSceneCtx;

void TelemetrySplashScene_cleanup(struct Scene *scene, Engine *UNUSED(engine)) {
    check(scene != NULL, "No scene to destroy");
error:
    return;
}

void TelemetrySplashScene_start(struct Scene *scene, Engine *engine) {
    TelemetrySplashSceneCtx *context =
        calloc(1, sizeof(TelemetrySplashSceneCtx));
    scene->context = context;
    context->line_color = VVector4ClearColor;
    context->text_color = VVector4ClearColor;
    context->extend = 0.0;

    char *font_path = engine->resource_path("fonts/uni.ttf");
    context->text_overlay = Overlay_create(engine, font_path, 24);
    free(font_path);
  
    VVector4 light_gray = {.raw = {0.9, 0.9, 0.9, 1.0}};
    scene->bg_color = light_gray;
}

void TelemetrySplashScene_stop(struct Scene *scene, Engine *UNUSED(engine)) {
    TelemetrySplashSceneCtx *context = (TelemetrySplashSceneCtx *)scene->context;
    Overlay_destroy(context->text_overlay);
  
    free(scene->context);
}

void TelemetrySplashScene_control(struct Scene *UNUSED(scene),
        Engine *UNUSED(engine)) { }

void TelemetrySplashScene_update(struct Scene *scene,
        Engine *engine) {
    float initial_delay = 1000.0;
    float color_fade = 1000.0;
    float text_fade = 2000.0;
    float total_length = 5000.0;
  
    TelemetrySplashSceneCtx *context = (TelemetrySplashSceneCtx *)scene->context;
    long int scene_ticks = Engine_get_ticks(engine) - scene->started_at;
  
    if (!context->finished) {
        if (scene_ticks >= total_length) {
            context->finished = 1;
        }
      
        if (engine->input->controllers[0]->jump) {
            context->finished = 1;
        }
      
        if (context->finished) {
            Scripting_call_hook(engine->scripting, scene, "finish_splash");
        }
    }
    if (scene_ticks < initial_delay) return;
  
    if (!context->sfx) {
        context->sfx = 1;
        char *spath = engine->resource_path("sfx/tlm_woosh.ogg");
        Sfx *sfx = Audio_gen_sfx(engine->audio, spath);
        Sfx_play(sfx);
    }
  
    float a = MIN((scene_ticks - initial_delay) / color_fade, 1);
    float na_eased = ease_out_cubic(a, 1.0, -1.0, 1.0);
    context->line_color.a = a;
    scene->camera->translation.x = AXIS_CENTERING.x * na_eased * 1.8;
    scene->camera->translation.y = -AXIS_CENTERING.y * na_eased;
  
    float extend_eased = ease_out_cubic((scene_ticks - initial_delay), 0.0, 1.0,
                                        1000);
    extend_eased = MIN(extend_eased, 1);
    context->extend = extend_eased;
  
    if (scene_ticks < initial_delay + color_fade) return;
    a = MIN((scene_ticks - initial_delay - color_fade) / text_fade, 1);
    context->text_color.a = a * 1;
}

static void draw_lines(Scene *scene, VVector3 *points, int num_points, Engine *engine) {
    TelemetrySplashSceneCtx *context = (TelemetrySplashSceneCtx *)scene->context;
    GLfloat color[4] = {context->line_color.r,
                        context->line_color.g,
                        context->line_color.b,
                        context->line_color.a};
    VMatrix tmvm = engine->graphics->modelview_matrix;

    VVector4 cVertex = {.raw = {color[0], color[1], color[2], color[3]}};
    VVector4 aVertex = {.raw = {1, 1, 1, 1}};
    VVector4 tex = {.raw = {0,0,0,0}};

    VVector4 vertices[8 * num_points];
    int i = 0;
    for (i = 0; i < num_points; i++) {
        int pos_idx   = i * 8;
        int color_idx = i * 8 + 1;
        int alpha_idx = i * 8 + 2;
        int tex_idx   = i * 8 + 3;
        int mvm_1_idx = i * 8 + 4;
        int mvm_2_idx = i * 8 + 5;
        int mvm_3_idx = i * 8 + 6;
        int mvm_4_idx = i * 8 + 7;

        VVector3 point = points[i];
        VVector4 pos = {.raw = {point.x, point.y, point.z, 1.0}};
        vertices[pos_idx] = pos;
        vertices[color_idx] = cVertex;
        vertices[alpha_idx] = aVertex;
        vertices[tex_idx] = tex;
        vertices[mvm_1_idx] = tmvm.v[0];
        vertices[mvm_2_idx] = tmvm.v[1];
        vertices[mvm_3_idx] = tmvm.v[2];
        vertices[mvm_4_idx] = tmvm.v[3];
    }
    glBufferData(GL_ARRAY_BUFFER, 8 * num_points * sizeof(VVector4), vertices,
            GL_STATIC_DRAW);
    glLineWidth(2);
    glDrawArrays(GL_LINES, 0, num_points);
}

#define TEL_AXIS_POINTS 6
static void draw_axis(Scene *scene, Engine *engine) {
    TelemetrySplashSceneCtx *context = (TelemetrySplashSceneCtx *)scene->context;
    int num_points = TEL_AXIS_POINTS;
    float z = -40;
    float zextend = 40;
    VVector3 points[TEL_AXIS_POINTS] = {
        // Y Axis
        {.v = {0, -500, z / 2}},
        {.v = {0, 500, z / 2}},
      
        // X Axis
        {.v = {-500, 0, z / 2}},
        {.v = {500, 0, z / 2}},
      
        // Z Axis
        {.v = {0, 0, z / 2 + 18 * context->extend}},
        {.v = {0, 0, z / 2 - zextend * context->extend}},
    };
  
    Graphics_translate_modelview_matrix(engine->graphics, AXIS_CENTERING.x,
                                        AXIS_CENTERING.y, 0);
    draw_lines(scene, points, num_points, engine);
}

void TelemetrySplashScene_render(struct Scene *scene, Engine *engine) {
    if (scene->started == 0) return;

    Graphics *graphics = ((Engine *)engine)->graphics;
    TelemetrySplashSceneCtx *context = (TelemetrySplashSceneCtx *)scene->context;
  
    GfxShader *dshader = Graphics_get_shader(graphics, "decal");

    Scene_fill(scene, engine, scene->bg_color);

    Graphics_use_shader(graphics, dshader);
    Camera *camera = scene->camera;
    // 3D!!!!!
    Graphics_reset_modelview_matrix(graphics);
    Graphics_reset_projection_matrix(graphics);
    float fov = 120;
    float aspect = camera->screen_size.w / camera->screen_size.h;
    float near = 1;
    float far = 200;
  
    double half_height = near * tan( fov * M_PI / 360.0 );
    double half_width = half_height * aspect;
    Graphics_frustum_projection_matrix(graphics,
                                       -half_width * 3 / 4, half_width / 4,
                                       -half_height / 4, half_height * 3.0 / 4,
                                       near, far);
  
    // Graphics_perspective_projection_matrix(graphics, fov, aspect, near, far);
  
    Graphics_scale_projection_matrix(graphics, camera->scale,
            camera->scale, 1);
    Graphics_rotate_projection_matrix(graphics,
                                      camera->rotation_radians * 180 / M_PI,
                                      0, 0, -1);
    Graphics_translate_projection_matrix(graphics,
                                         -camera->translation.x,
                                         camera->translation.y,
                                         0);
    Graphics_uniformMatrix4fv(graphics,
                              UNIFORM_DECAL_PROJECTION_MATRIX,
                              engine->graphics->projection_matrix.gl,
                              GL_FALSE);
    draw_axis(scene, engine);

    Graphics_project_screen_camera(engine->graphics, scene->camera);
    GfxShader *txshader = Graphics_get_shader(engine->graphics, "text");
    Graphics_use_shader(engine->graphics, txshader);
    Graphics_reset_modelview_matrix(engine->graphics);
    VVector4 clear = VVector4ClearColor;
    VPoint txt_origin = {scene->camera->screen_size.w / 2 - 10,
        scene->camera->screen_size.h / 2 - 10};
    Graphics_draw_string(engine->graphics, "TELEMETRY", context->text_overlay->font,
            context->text_color.raw, txt_origin, GfxTextAlignRight, clear.raw, VPointZero);

    Scene_fill(scene, engine, scene->cover_color);
}

SceneProto TelemetrySplashSceneProto = {
    .start = TelemetrySplashScene_start,
    .start_success_cb = NULL,
    .stop = TelemetrySplashScene_stop,
    .cleanup = TelemetrySplashScene_cleanup,
    .update = TelemetrySplashScene_update,
    .render = TelemetrySplashScene_render,
    .control = TelemetrySplashScene_control,
    .add_entity_cb = NULL,
    .remove_entity_cb = NULL,
    .get_gravity = NULL,
    .set_gravity = NULL
};
