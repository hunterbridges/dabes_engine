#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SDL/SDL.h"
#include "SDL/SDL_opengl.h"
#include "SDL_TTF.h"
#include "dbg.h"
#include "gameobjects.h"

#define FPS 60
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define NUM_BOXES 10

Object ThingProto = {
   .init = Thing_init,
   .calc_physics = Thing_calc_physics
};

int initGL() {
    glEnable(GL_TEXTURE_2D);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glClearColor(0.f, 0.f, 0.f, 1.f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(SCREEN_WIDTH / 2.f, SCREEN_HEIGHT / 2.f, 0.f);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glShadeModel (GL_FLAT);
    GLenum error = glGetError();
    check(error == GL_NO_ERROR, "OpenGL init error...");
    return 1;
error:
    printf("Error initializing OpenGL! %s\n", gluErrorString(error));
    return 0;
}

int loadBgTexture(SDL_Surface *surface) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, surface->w, surface->h,
        0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //glBindTexture(GL_TEXTURE_2D, 0);
    SDL_FreeSurface(surface);
    return texture;
}

int Game_init(void *self) {
    check_mem(self);

    Game *game = (Game *)self;
    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        Thing *thing = NEW(Thing, "A thing");
        thing->width = SCREEN_WIDTH / (2 * NUM_BOXES);
        thing->height = thing->width;
        thing->x = i * (SCREEN_WIDTH / NUM_BOXES) + thing->width / 2;
        thing->y = i;
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
    float swidth = SCREEN_WIDTH;
    float sheight = SCREEN_HEIGHT;

    SDL_Event event = {};
    SDL_Surface *screen = NULL;

    SDL_Init(SDL_INIT_EVERYTHING);
    TTF_Init();

    TTF_Font *font = TTF_OpenFont("uni.ttf", 8);

    Game *game = NEW(Game, "The game");

    screen = SDL_SetVideoMode((int)swidth, (int)sheight, 32, SDL_OPENGLBLIT);
    Uint32 black = SDL_MapRGBA(screen->format, 0, 0, 0, 255);

    check(initGL() == 1, "Init OpenGL");

    SDL_Surface *bg = graydient(640, 480);
    int texture = loadBgTexture(bg);

    int skip = 1000 / FPS;

    long unsigned int last_frame_at = 0;
    short int init = 0;

    float xadj = -1;
    float yadj = 1;
    float gProjectionScale = 1.0;
    float gRotation = 0;
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
                gProjectionScale = 1;
                gRotation = 0;
                reset = 0;
            }
            gProjectionScale += 0.01 * zoom;
            gRotation += 1 * rot;
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            glTranslatef(-SCREEN_WIDTH / 2.f,-SCREEN_HEIGHT / 2.f, 0.f );

            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            glOrtho(SCREEN_WIDTH / -2 * gProjectionScale,
                    SCREEN_WIDTH / 2 * gProjectionScale,
                    SCREEN_HEIGHT / 2 * gProjectionScale,
                    SCREEN_HEIGHT / -2 * gProjectionScale,
                    1.0, -1.0 );
            glRotatef(gRotation, 0, 0, -1);
            glClear(GL_COLOR_BUFFER_BIT);
            int i = 0;

            SDL_Rect debugRect = {10, 10, 200, 100};
            glBindTexture(GL_TEXTURE_2D, texture);
            glBegin(GL_QUADS);
                glColor3f(1.f, 1.f, 1.f);
                glTexCoord2f(0, 0);
                glVertex2f(0, 0);
                glTexCoord2f(1, 0);
                glVertex2f(SCREEN_WIDTH, 0);
                glTexCoord2f(1, 1);
                glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
                glTexCoord2f(0, 1);
                glVertex2f(0, SCREEN_HEIGHT);
            glEnd();
            glBindTexture(GL_TEXTURE_2D, 0);
            //SDL_BlitSurface(bg, &debugRect, screen, &debugRect);

            game->_(calc_physics)(game, ticks_since_last);

            for (i = 0; i < NUM_BOXES; i++) {
                Thing *thing = game->things[i];
                if (thing == NULL) break;
                SDL_Rect rect = thing->rect(thing);
                glBegin(GL_QUADS);
                    glColor3f( 0.f, 0.f, 0.f );
                    glVertex2f(rect.x,
                               rect.y);
                    glVertex2f((rect.x + rect.w),
                               rect.y);
                    glVertex2f((rect.x + rect.w),
                               (rect.y + rect.h));
                    glVertex2f((rect.x),
                               (rect.y + rect.h));
                glEnd();
                //SDL_FillRect(screen, &rect, black);
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

            SDL_GL_SwapBuffers();
            last_frame_at = ticks;
        }
    }

    glDeleteTextures(1, &texture);
    game->_(destroy)(game);
    //SDL_FreeSurface(bg);
    SDL_FreeSurface(screen);
    TTF_CloseFont(font);

    return 0;
error:
    game->_(destroy)(game);
    SDL_FreeSurface(screen);
    TTF_CloseFont(font);
    return 1;
}
