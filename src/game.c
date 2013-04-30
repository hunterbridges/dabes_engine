#include "prefix.h"
#include "gameobjects.h"
#include "scenes/ortho_chipmunk_scene.h"
#include "scenes/ortho_physics_scene.h"

int main(int argc, char *argv[]) {
    argc = (int)argc;
    argv = (char **)argv;

    Engine *engine = NULL;
    SDL_Surface *screen = NULL;
    Scene *scene = NULL;

    engine = Engine_create("media/scripts/boxfall.lua", (void *)&screen);
    check(engine != NULL, "Failed to boot engine");

    scene = Scene_create(engine, OrthoChipmunkSceneProto, "fat_map");

    GameEntity_assign_controller(scene->entities->first->value,
            engine->input->controllers[0]);

    while (engine->input->game_quit == 0) {
        Engine_regulate(engine);
        Input_poll(engine->input);
        Audio_stream(engine->audio);

        if (engine->frame_now) {
            scene->_(control)(scene, engine);
            scene->_(update)(scene, engine);
            scene->_(render)(scene, engine);
#ifdef DEBUG
            Graphics_draw_debug_text(engine->graphics, engine->frame_ticks);
#endif
            SDL_GL_SwapBuffers();

            if (engine->input->game_quit) break;
            Input_reset(engine->input);
        }
    }

    Scene_destroy(scene, engine);
    Engine_destroy(engine);
    SDL_FreeSurface(screen);

    return 0;
error:
    if (scene) Scene_destroy(scene, engine);
    if (engine) engine->_(destroy)(engine);
    SDL_FreeSurface(screen);
    return 1;
}
