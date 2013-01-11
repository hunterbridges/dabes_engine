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

SDL_Surface *graydient(unsigned int width, unsigned int height) {
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_HWSURFACE, width, height, 32, rmask, gmask, bmask, amask);
    int i = 0;
    for (i = 0; i < height; i++) {
        int gradation = 255 * (height - i) / height;
        Uint32 color = SDL_MapRGBA(surface->format, 0, 255 - gradation, gradation, 255);
        SDL_Rect rect = {0, i, width, 1};
        SDL_FillRect(surface, &rect, color);
    }
    return surface;
}

int main(int argc, char *argv[])
{
    int quit = 0;
    Uint32 start = 0;

    SDL_Event event = {};
    SDL_Surface *screen = NULL;

    SDL_Init(SDL_INIT_EVERYTHING);

    Game *game = NEW(Game, "The game");

    screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    Uint32 white = SDL_MapRGBA(screen->format, 255, 255, 255, 255);
    Uint32 black = SDL_MapRGBA(screen->format, 0, 0, 0, 255);

    SDL_Surface *bg = graydient(640, 480);

    int skip = 1000 / FPS;

    long unsigned int last_frame_at = 0;
    short int init = 0;
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

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_q) {
                    quit = 1;
                }
            }
        }

        if (frame) {
            game->_(calc_physics)(game, ticks_since_last);

            SDL_Rect fullscreen = {0, 0, 640, 480};
            SDL_Rect rect = {game->thing->x, game->thing->y, 100, 100};

            SDL_BlitSurface(bg, NULL, screen, NULL);
            SDL_FillRect(screen, &rect, black);

            SDL_Flip(screen);
            last_frame_at = ticks;
        }
    }

    game->_(destroy)(game);
    free(bg);

    return 1;
}
