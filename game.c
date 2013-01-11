#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "dbg.h"
#include "gameobjects.h"

#define FPS 60

Object ThingProto = {
   .init = Thing_init,
   .calc_physics = Thing_calc_physics
};

int Game_init(void *self) {
    check_mem(self);

    Game *game = (Game *)self;
    game->thing = NEW(Thing, "A thing");

    return 1;
error:
    return 0;
}

Object GameProto = {
   .init = Game_init,
   .calc_physics = Game_calc_physics,
   .destroy = Game_destroy
};

int main(int argc, char *argv[])
{
    int quit = 0;
    Uint32 start = 0;

    SDL_Event event = {};
    SDL_Surface *hello = NULL;
    SDL_Surface *screen = NULL;

    SDL_Init(SDL_INIT_EVERYTHING);
    hello = SDL_LoadBMP("hello.bmp");

    Game *game = NEW(Game, "The game");

    screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    Uint32 white = SDL_MapRGBA(screen->format, 255, 255, 255, 255);
    Uint32 black = SDL_MapRGBA(screen->format, 0, 0, 0, 255);

    int skip = 1000 / FPS;

    int last = 0;
    int init = 0;
    while (quit == 0) {
        int ticks = SDL_GetTicks();
        int frames = 0;
        if (init == 0) {
            debug("frame");
            start = ticks;
            last = ticks;
            init = 1;
            frames = 1;
        } else {
            if (ticks - last >= skip) {
                frames = (ticks - last) / skip;
            }
        }

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_q) {
                    quit = 1;
                }
            }
        }

        int frame = 0;
        if (frames > 0) {
            game->_(calc_physics)(game, ticks - last);

            SDL_Rect fullscreen = {0, 0, 640, 480};
            SDL_Rect rect = {game->thing->x, game->thing->y, 100, 100};
            SDL_FillRect(screen, &fullscreen, white);
            SDL_FillRect(screen, &rect, black);

            SDL_Flip(screen);
            last = ticks;
        }
    }

    game->_(destroy)(game);

    return 1;
}
