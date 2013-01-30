#include "prefix.h"
#include "gameobjects.h"
#include "SDL/SDL_TTF.h"

int main(int argc, char *argv[])
{
    argc = (int)argc;
    argv = (char **)argv;

    float swidth = SCREEN_WIDTH;
    float sheight = SCREEN_HEIGHT;
    Engine *engine = NULL;
    Scene *game = NULL;

    SDL_Event event = {};
    SDL_Surface *screen = NULL;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    TTF_Init();
    TTF_Font *font = TTF_OpenFont("media/fonts/uni.ttf", 8);

    screen = SDL_SetVideoMode((int)swidth, (int)sheight, 32, SDL_OPENGL);

    check(initGL(SCREEN_WIDTH, SCREEN_HEIGHT) == 1, "Init OpenGL");

    engine = NEW(Engine, "The game engine");
    game = NEW(Scene, "The game");

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
                game->projection_scale = 1;
                game->projection_rotation = 0;
                reset = 0;
            }
            game->projection_scale += 0.02 * zoom;
            if (game->projection_scale < 0) game->projection_scale = 0;

            float volume = game->projection_scale * 128.f;
            Mix_VolumeMusic(volume);

            game->projection_rotation += 2 * rot;

            game->_(calc_physics)(game, engine, ticks_since_last);
            game->_(render)(game, engine);

            // TODO: Make the debug text work again
            SDL_Rect debugRect = {10, 10, 200, 100};
            SDL_Color txtBlack = {0,0,0,255};
            char *dTxt = malloc(256 * sizeof(char));
            sprintf(dTxt, "FPS CAP: %d           ACTUAL: %.2f", FPS,
                    1000.0 / ticks_since_last);
            SDL_Surface *debugText = TTF_RenderText_Solid(font,
                    dTxt, txtBlack);
            free(dTxt);

            SDL_BlitSurface(debugText, NULL, screen, &debugRect);
            SDL_FreeSurface(debugText);

            SDL_GL_SwapBuffers();
            last_frame_at = ticks;
        }
    }

    game->_(destroy)(game);
    engine->_(destroy)(engine);
    SDL_FreeSurface(screen);
    TTF_CloseFont(font);

    return 0;
error:
    if (game) game->_(destroy)(game);
    if (engine) engine->_(destroy)(engine);
    SDL_FreeSurface(screen);
    TTF_CloseFont(font);
    return 1;
}
