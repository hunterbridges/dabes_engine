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

    return 1;
error:
    return 0;
}


Object SceneProto = {
   .init = Scene_init,
   .calc_physics = Scene_calc_physics,
   .destroy = Scene_destroy
};
