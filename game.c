#include <stdio.h>
#include <stdlib.h>
#include "SDL/SDL.h"
#include "SDL_TTF.h"
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
    int i = 0;
    for (i = 0; i < 200; i++) {
        Thing *thing = NEW(Thing, "A thing");
        thing->x = i * (640 / 200);
        thing->y = i;
        thing->width = 2;
        thing->height = 2;
        thing->mass = 1000;
        thing->time_scale = (i + 1) / 400.0;
        game->things[i] = thing;
    }

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

    SDL_Surface *surface = SDL_CreateRGBSurface(SDL_HWSURFACE, width,
            height, 32, rmask, gmask, bmask, amask);
    int i = 0;
    for (i = 0; i < height; i++) {
        int gradation = 255 * (height - i) / height;
        Uint32 color = SDL_MapRGBA(surface->format, 0,
                255 - gradation, gradation, 255);
        SDL_Rect rect = {0, i, width, 1};
        SDL_FillRect(surface, &rect, color);
    }
    return surface;
}

int main(int argc, char *argv[])
{
    int quit = 0;

    SDL_Event event = {};
    SDL_Surface *screen = NULL;

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    TTF_Font *font = TTF_OpenFont("uni.ttf", 8);

    Game *game = NEW(Game, "The game");

    screen = SDL_SetVideoMode(640, 480, 32, SDL_HWSURFACE);
    Uint32 black = SDL_MapRGBA(screen->format, 0, 0, 0, 255);

    SDL_Surface *bg = graydient(640, 480);

    int skip = 1000 / FPS;

    long unsigned int last_frame_at = 0;
    short int init = 0;
    SDL_BlitSurface(bg, NULL, screen, NULL);

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
            int i = 0;
            for (i = 0; i < 256; i++) {
                Thing *thing = game->things[i];
                if (thing == NULL) break;
                SDL_Rect rect = thing->rect(thing);
                SDL_BlitSurface(bg, &rect, screen, &rect);
            }

            SDL_Rect debugRect = {10, 10, 200, 100};
            SDL_BlitSurface(bg, &debugRect, screen, &debugRect);

            game->_(calc_physics)(game, ticks_since_last);

            for (i = 0; i < 256; i++) {
                Thing *thing = game->things[i];
                if (thing == NULL) break;
                SDL_Rect rect = thing->rect(thing);
                SDL_FillRect(screen, &rect, black);
            }

            SDL_Color txtBlack = {0,0,0,255};
            char *dTxt = malloc(256 * sizeof(char));
            sprintf(dTxt, "FPS CAP: %d           ACTUAL: %.2f", FPS,
                    1000.0 / ticks_since_last);
            SDL_Surface *debugText = TTF_RenderText_Solid(font,
                    dTxt, txtBlack);
            free(dTxt);
            SDL_BlitSurface(debugText, NULL, screen, &debugRect);
            SDL_FreeSurface(debugText);

            SDL_Flip(screen);
            last_frame_at = ticks;
        }
    }

    game->_(destroy)(game);
    SDL_FreeSurface(bg);
    SDL_FreeSurface(screen);
    TTF_CloseFont(font);

    return 1;
}
