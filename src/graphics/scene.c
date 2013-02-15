#include "../core/engine.h"
#include "scene.h"

void Scene_destroy(void *self) {
    check_mem(self);
    Scene *game = (Scene *)self;

    Mix_HaltMusic();
    Mix_FreeMusic(game->music);

    int i = 0;
    for (i = 0; i < 256; i++) {
        if (game->entities[i] == NULL) break;
        GameEntity *thing = game->entities[i];
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
        GameEntity *entity = NEW(GameEntity, "A thing");
        entity->texture = load_image_as_texture("media/sprites/dumblock.png");
        game->entities[i] = entity;
        entity->alpha = (double)i / NUM_BOXES * 1.0;
    }

    SDL_Surface *bg = gradient(640, 480);
    game->bg_texture = load_surface_as_texture(bg);

    game->projection_scale = 1;
    game->projection_rotation = 0;

    return 1;
error:
    return 0;
}

void Scene_render(void *self, void *engine) {
    Scene *game = (Scene *)self;
    Graphics *graphics = ((Engine *)engine)->graphics;

    glClear(GL_COLOR_BUFFER_BIT);

    double bgScale = (game->projection_scale + 2) / 2;
    Graphics_scale_projection_matrix(graphics, bgScale);
    GfxRect gfx_rect = GfxRect_from_xywh(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    GLdouble color[4] = {1.f, 1.f, 1.f, 1.f};
    Graphics_draw_rect(graphics, gfx_rect, color, game->bg_texture, 0);

    Graphics_scale_projection_matrix(graphics, game->projection_scale);
    glRotatef(game->projection_rotation, 0, 0, -1);

    // Draw the stuff
    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *thing = game->entities[i];
        if (thing == NULL) break;
        GameEntity_render(thing, engine);
    }
}

World *Scene_create_world(Scene *scene, Physics *physics) {
    check_mem(scene);
    check_mem(physics);

    World *world = NEW(World, "The world");
    check_mem(world);

    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *entity = scene->entities[i];
        if (entity == NULL) break;

        Fixture *fixture = World_create_fixture(world);
        fixture->width =  3;
        fixture->height = fixture->width;// + fixture->width * 2 * i / NUM_BOXES;
        fixture->center.x = (world->width - (NUM_BOXES - i) * fixture->width * 4 + 2 * (NUM_BOXES + 1) * fixture->width) / 2;
        fixture->center.y = 1;
        fixture->time_scale = 1;
        fixture->rotation_radians = M_PI / 16 * (i % 8);
        Fixture_set_mass(fixture, 10);

        entity->fixture = fixture;
    }
    return world;
error:
    return NULL;
}

void Scene_control(Scene *scene, Input *input) {
    if (input->cam_reset) {
        scene->projection_scale = 1;
        scene->projection_rotation = 0;
    }
    scene->projection_scale += 0.02 * input->cam_zoom;
    if (scene->projection_scale < 0) scene->projection_scale = 0;

    scene->projection_rotation += 2 * input->cam_rotate;

    double volume = scene->projection_scale * 128.f;
    Mix_VolumeMusic(volume);

    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *entity = scene->entities[i];
        GameEntity_control(entity, input);
    }
}

Object SceneProto = {
   .init = Scene_init,
   .destroy = Scene_destroy
};

