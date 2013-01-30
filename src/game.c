#include "prefix.h"
#include "gameobjects.h"
#include "graphics.h"
#include "SDL/SDL_TTF.h"

int main(int argc, char *argv[])
{
    argc = (int)argc;
    argv = (char **)argv;

    float swidth = SCREEN_WIDTH;
    float sheight = SCREEN_HEIGHT;

    SDL_Event event = {};
    SDL_Surface *screen = NULL;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    TTF_Init();
    TTF_Font *font = TTF_OpenFont("media/fonts/uni.ttf", 8);

    Scene *game = NEW(Scene, "The game");

    screen = SDL_SetVideoMode((int)swidth, (int)sheight, 32, SDL_OPENGLBLIT);

    check(initGL(SCREEN_WIDTH, SCREEN_HEIGHT) == 1, "Init OpenGL");

    SDL_Surface *bg = gradient(640, 480);
    GLuint texture = loadSurfaceAsTexture(bg);

    int skip = 1000 / FPS;

    long unsigned int last_frame_at = 0;
    short int init = 0;

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
            glBegin(GL_TRIANGLE_STRIP);
                glColor3f(1.f, 1.f, 1.f);
                glTexCoord2f(0, 0);
                glVertex2f(0, 0);
                glTexCoord2f(1, 0);
                glVertex2f(SCREEN_WIDTH, 0);
                glTexCoord2f(0, 1);
                glVertex2f(0, SCREEN_HEIGHT);
                glTexCoord2f(1, 1);
                glVertex2f(SCREEN_WIDTH, SCREEN_HEIGHT);
            glEnd();
            glBindTexture(GL_TEXTURE_2D, 0);

            game->_(calc_physics)(game, ticks_since_last);

            for (i = 0; i < NUM_BOXES; i++) {
                GameEntity *thing = game->things[i];
                if (thing == NULL) break;
                SDL_Rect rect = thing->rect(thing);
                glBegin(GL_TRIANGLE_STRIP);
                    glColor3f( 0.f, 0.f, 0.f );
                    glVertex2f(rect.x,
                               rect.y);
                    glVertex2f((rect.x + rect.w),
                               rect.y);
                    glVertex2f((rect.x),
                               (rect.y + rect.h));
                    glVertex2f((rect.x + rect.w),
                               (rect.y + rect.h));
                glEnd();
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
    SDL_FreeSurface(screen);
    TTF_CloseFont(font);

    return 0;
error:
    game->_(destroy)(game);
    SDL_FreeSurface(screen);
    TTF_CloseFont(font);
    return 1;
}
