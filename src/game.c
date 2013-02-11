#include "prefix.h"
#include "gameobjects.h"
#include "SDL/SDL_TTF.h"

int main(int argc, char *argv[]) {
    argc = (int)argc;
    argv = (char **)argv;

    double swidth = SCREEN_WIDTH;
    double sheight = SCREEN_HEIGHT;
    Engine *engine = NULL;
    Scene *scene = NULL;
    World *world = NULL;

    SDL_Surface *screen = NULL;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    TTF_Init();

    screen = SDL_SetVideoMode((int)swidth, (int)sheight, 32, SDL_OPENGL);

    check(init_GL(SCREEN_WIDTH, SCREEN_HEIGHT) == 1, "Init OpenGL");

    engine = NEW(Engine, "The game engine");
    scene = NEW(Scene, "The game");
    world = Scene_create_world(scene, engine->physics);

    //TODO: Put this in a civilized place.
    scene->entities[0]->controller = engine->input->controllers[0];

    int skip = 1000 / FPS;

    long unsigned int last_frame_at = 0;
    short int init = 0;

    while (engine->input->game_quit == 0) {
        long unsigned int ticks = SDL_GetTicks();
        long unsigned int ticks_since_last = ticks - last_frame_at;
        short int frame = 0;

        if (init == 0) {
            last_frame_at = ticks;
            init = 1;
            frame = 1;
        } else {
            if (ticks_since_last >= skip) {
                frame = 1;
            }
        }

        Input_poll(engine->input);

        if (frame) {
            Scene_control(scene, engine->input);

            World_solve(engine->physics, world, ticks_since_last);
            Scene_render(scene, engine);
#ifdef DEBUG
            Graphics_draw_debug_text(engine->graphics, ticks_since_last);
#endif
            SDL_GL_SwapBuffers();

            last_frame_at = ticks;
            if (engine->input->game_quit) break;
            Input_reset(engine->input);
        }
    }

    scene->_(destroy)(scene);
    engine->_(destroy)(engine);
    SDL_FreeSurface(screen);

    return 0;
error:
    if (scene) scene->_(destroy)(scene);
    if (engine) engine->_(destroy)(engine);
    SDL_FreeSurface(screen);
    return 1;
}
