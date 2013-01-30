#include "engine.h"
#include "scene.h"

void Scene_destroy(void *self) {
    check_mem(self);
    Scene *game = (Scene *)self;

    Mix_HaltMusic();
    Mix_FreeMusic(game->music);

    int i = 0;
    for (i = 0; i < 256; i++) {
        if (game->things[i] == NULL) break;
        GameEntity *thing = game->things[i];
        thing->_(destroy)(thing);
    }

    glDeleteTextures(1, &game->bg_texture);
    Object_destroy(self);
error:
    return;
}

int Scene_init(void *self) {
    check_mem(self);

    Scene *game = (Scene *)self;
    game->music = Mix_LoadMUS("media/music/tower.wav");

    if (game->music == NULL) {
        printf("Mix_LoadMUS: %s\n", Mix_GetError());
    }

    Mix_PlayMusic(game->music, -1);

    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *thing = NEW(GameEntity, "A thing");
        thing->width = SCREEN_WIDTH / (2 * NUM_BOXES);
        thing->height = thing->width;
        thing->x = i * (SCREEN_WIDTH / NUM_BOXES) + thing->width;
        thing->y = i;
        thing->time_scale = (i + 1) / 400.0;
        game->things[i] = thing;
    }

    SDL_Surface *bg = gradient(640, 480);
    game->bg_texture = loadSurfaceAsTexture(bg);

    game->projection_scale = 1;
    game->projection_rotation = 0;

    return 1;
error:
    return 0;
}

void Scene_calc_physics(void *self, void *engine, int ticks) {
    check_mem(self);
    Scene *game = (Scene *)self;
    int i = 0;
    for (i = 0; i < 256; i++) {
        if (game->things[i] == NULL) break;
        GameEntity *thing = game->things[i];
        thing->_(calc_physics)(thing, engine, ticks);
    }

error:
    return;
}

void Scene_render(void *self, void *engine) {
    Scene *game = (Scene *)self;
    Graphics *graphics = ((Engine *)engine)->graphics;

    glClear(GL_COLOR_BUFFER_BIT);

    float bgScale = (game->projection_scale + 2) / 2;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(SCREEN_WIDTH / (-2 * bgScale),
            SCREEN_WIDTH / (2 * bgScale),
            SCREEN_HEIGHT / (2 * bgScale),
            SCREEN_HEIGHT / (-2 * bgScale),
            1.0, -1.0 );

    // Draw the background
    SDL_Rect rect = {0, 0,
        SCREEN_WIDTH, SCREEN_HEIGHT};
    GLfloat color[3] = {1.f, 1.f, 1.f};
    graphics->draw_rect(graphics, rect, color, game->bg_texture, 0);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(SCREEN_WIDTH / (-2 * game->projection_scale),
            SCREEN_WIDTH / (2 * game->projection_scale),
            SCREEN_HEIGHT / (2 * game->projection_scale),
            SCREEN_HEIGHT / (-2 * game->projection_scale),
            1.0, -1.0 );
    glRotatef(game->projection_rotation, 0, 0, -1);

    // Draw the stuff
    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *thing = game->things[i];
        if (thing == NULL) break;
        thing->_(render)(thing, engine);
    }
}

Object SceneProto = {
   .init = Scene_init,
   .calc_physics = Scene_calc_physics,
   .destroy = Scene_destroy,
   .render = Scene_render
};

