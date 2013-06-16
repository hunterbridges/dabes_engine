#include "prefix.h"
#include "gameobjects.h"
#include "scenes/ortho_chipmunk_scene.h"

int main(int argc, char *argv[]) {
    argc = (int)argc;
    argv = (char **)argv;

    Engine *engine = NULL;
    SDL_Surface *screen = NULL;
    Scene *scene = NULL;

    engine = Engine_create("scripts/boxfall/boot.lua", (void *)&screen);
    check(engine != NULL, "Failed to boot engine");

    Scripting_boot(engine->scripting);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    while (engine->input->game_quit == 0) {
        Engine_regulate(engine);
        Input_poll(engine->input);
        Audio_stream(engine->audio);

        if (engine->frame_now) {
            scene = Engine_get_current_scene(engine);
            if (scene) {
                scene->_(update)(scene, engine);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                scene->_(render)(scene, engine);
            }
#ifdef DEBUG
            Graphics_draw_debug_text(engine->graphics, engine->frame_ticks);
#endif
            SDL_GL_SwapBuffers();

            if (engine->input->game_quit) break;
            Input_reset(engine->input);
            Engine_frame_end(engine);
        }
    }

    Engine_destroy(engine);
    SDL_FreeSurface(screen);

    return 0;
error:
    if (engine) engine->_(destroy)(engine);
    SDL_FreeSurface(screen);
    return 1;
}
