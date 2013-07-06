#include "prefix.h"
#include "gameobjects.h"
#include "scenes/ortho_chipmunk_scene.h"

char *Boxfall_project_path(const char *filename) {
  const char *boxfall = "demo/boxfall/";
  char *newpath = calloc(strlen(boxfall) + strlen(filename) + 1, sizeof(char));
  strcpy(newpath, boxfall);
  strcat(newpath, filename);
  return newpath;
}

int main(int argc, char *argv[]) {
    argc = (int)argc;
    argv = (char **)argv;

    Engine *engine = NULL;
    SDL_Surface *screen = NULL;
    Scene *scene = NULL;

    engine = Engine_create(NULL, Boxfall_project_path,
            "scripts/boot.lua", (void *)&screen);
    check(engine != NULL, "Failed to boot engine");

    Scripting_boot(engine->scripting);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    while (engine->input->game_quit == 0) {
        Engine_regulate(engine);
        Input_poll(engine->input);
        Audio_stream(engine->audio);

        if (engine->frame_now) {
            Engine_update_easers(engine);
            scene = Engine_get_current_scene(engine);
            if (scene) {
                Scene_update(scene, engine);
                if (scene->started) {
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    Scene_render(scene, engine);
                }
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
    Engine_destroy(engine);
    SDL_FreeSurface(screen);
    return 1;
}
