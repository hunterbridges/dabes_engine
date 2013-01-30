#include "scene.h"

void Scene_calc_physics(void *self, int ticks) {
    check_mem(self);
    Scene *game = (Scene *)self;
    int i = 0;
    for (i = 0; i < 256; i++) {
        if (game->things[i] == NULL) break;
        GameEntity *thing = game->things[i];
        thing->_(calc_physics)(thing, ticks);
    }

error:
    return;
}

void Scene_destroy(void *self) {
    check_mem(self);
    Scene *game = (Scene *)self;
    int i = 0;
    for (i = 0; i < 256; i++) {
        if (game->things[i] == NULL) break;
        GameEntity *thing = game->things[i];
        thing->_(destroy)(thing);
    }

    glDeleteTextures(1, &game->bgTexture);
    Object_destroy(self);
error:
    return;
}

int Scene_init(void *self) {
    check_mem(self);

    Scene *game = (Scene *)self;
    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *thing = NEW(GameEntity, "A thing");
        thing->width = SCREEN_WIDTH / (2 * NUM_BOXES);
        thing->height = thing->width;
        thing->x = i * (SCREEN_WIDTH / NUM_BOXES) + thing->width / 2;
        thing->y = i;
        thing->mass = 1000;
        thing->time_scale = (i + 1) / 400.0;
        game->things[i] = thing;
    }

    SDL_Surface *bg = gradient(640, 480);
    game->bgTexture = loadSurfaceAsTexture(bg);

    return 1;
error:
    return 0;
}

void Scene_render(void *self) {
    Scene *game = (Scene *)self;
    glBindTexture(GL_TEXTURE_2D, game->bgTexture);
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

    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *thing = game->things[i];
        if (thing == NULL) break;
        thing->_(render)(thing);
    }
}

Object SceneProto = {
   .init = Scene_init,
   .calc_physics = Scene_calc_physics,
   .destroy = Scene_destroy,
   .render = Scene_render
};

