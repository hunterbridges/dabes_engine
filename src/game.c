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

    SDL_Event event = {};
    SDL_Surface *screen = NULL;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    TTF_Init();

    screen = SDL_SetVideoMode((int)swidth, (int)sheight, 32, SDL_OPENGL);

    check(initGL(SCREEN_WIDTH, SCREEN_HEIGHT) == 1, "Init OpenGL");

    engine = NEW(Engine, "The game engine");
    scene = NEW(Scene, "The game");
    world = Scene_create_world(scene, engine->physics);

    int skip = 1000 / FPS;

    long unsigned int last_frame_at = 0;
    short int init = 0;

    int reset = 0;
    int quit = 0;
    int zoom = 0;
    int rot = 0;
    while (quit == 0) {
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

        //TODO: Abstract into control handler
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_q) quit = 1;
                if (event.key.keysym.sym == SDLK_r) reset = 1;
                if (event.key.keysym.sym == SDLK_j) zoom += 1;
                if (event.key.keysym.sym == SDLK_k) zoom -= 1;
                if (event.key.keysym.sym == SDLK_u) rot += 1;
                if (event.key.keysym.sym == SDLK_i) rot -= 1;
            }
            if (event.type == SDL_KEYUP) {
                if (event.key.keysym.sym == SDLK_j) zoom -= 1;
                if (event.key.keysym.sym == SDLK_k) zoom += 1;
                if (event.key.keysym.sym == SDLK_u) rot -= 1;
                if (event.key.keysym.sym == SDLK_i) rot += 1;
            }
        }

        if (frame) {
            if (reset) {
                scene->projection_scale = 1;
                scene->projection_rotation = 0;
                reset = 0;
            }
            scene->projection_scale += 0.02 * zoom;
            if (scene->projection_scale < 0) scene->projection_scale = 0;
            scene->projection_rotation += 2 * rot;

            double volume = scene->projection_scale * 128.f;
            Mix_VolumeMusic(volume);

            World_solve(engine->physics, world, ticks_since_last);
            Scene_render(scene, engine);
#ifdef DEBUG
            Graphics_draw_debug_text(engine->graphics, ticks_since_last);
#endif

            SDL_GL_SwapBuffers();
            last_frame_at = ticks;
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
