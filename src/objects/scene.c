#include "engine.h"
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
        game->entities[i] = entity;
        entity->alpha = (float)i / NUM_BOXES * 0.75 + 0.25;
    }

    SDL_Surface *bg = gradient(640, 480);
    game->bg_texture = loadSurfaceAsTexture(bg);

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

    float bgScale = (game->projection_scale + 2) / 2;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(SCREEN_WIDTH / (-2 * bgScale),
            SCREEN_WIDTH / (2 * bgScale),
            SCREEN_HEIGHT / (2 * bgScale),
            SCREEN_HEIGHT / (-2 * bgScale),
            1.0, -1.0 );

    // Draw the background
    GfxRect gfx_rect = GfxRect_from_xywh(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    GLfloat color[4] = {1.f, 1.f, 1.f, 1.f};
    Graphics_draw_rect(graphics, gfx_rect, color, game->bg_texture, 0);

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
        GameEntity *thing = game->entities[i];
        if (thing == NULL) break;
        thing->_(render)(thing, engine);
    }
}

World *Scene_create_world(Scene *scene, Physics *physics) {
    check_mem(scene);

    World *world = NEW(World, "The world");
    check_mem(world);

    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *entity = scene->entities[i];
        if (entity == NULL) break;

        Fixture *fixture = World_create_fixture(world);
        fixture->width =  world->width / (2 * 10/*NUM_BOXES*/);
        fixture->height = fixture->width;// + fixture->width * 2 * i / NUM_BOXES;
        fixture->x = (world->width - (NUM_BOXES - i) * fixture->width * 4 + 2 * (NUM_BOXES + 1) * fixture->width) / 2;
        fixture->y = 1;
        fixture->time_scale = (float)(i + 1) / (NUM_BOXES + 1);
        fixture->time_scale = 1;
        fixture->rotation_radians = M_PI / 16 * (i % 8);
        Fixture_set_mass(fixture, 10 * (i + 1));

        fixture->spring.x = fixture->x - fixture->width / 2;

        entity->fixture = fixture;
    }
    return world;
error:
    return NULL;
}

Object SceneProto = {
   .init = Scene_init,
   .destroy = Scene_destroy,
   .render = Scene_render
};
