#include "../core/engine.h"
#include "scene.h"

void Scene_draw_debug_grid(Scene *scene, Graphics *graphics);

void Scene_destroy(void *self) {
    check_mem(self);
    Scene *game = (Scene *)self;

    Mix_HaltMusic();
    Mix_FreeMusic(game->music);

    LIST_FOREACH(game->entities, first, next, current) {
        GameEntity *thing = current->value;
        thing->_(destroy)(thing);
    }

    List_destroy(game->entities);
    if (game->world) game->world->_(destroy)(game->world);
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

    game->world = NULL;
    game->entities = List_create();

    int i = 0;
    for (i = 0; i < NUM_BOXES; i++) {
        GameEntity *entity = NEW(GameEntity, "A thing");
        entity->texture = load_image_as_texture("media/sprites/dumblock.png");
        List_push(game->entities, entity);
        entity->alpha = (double)i / NUM_BOXES * 1.0;
    }

    game->bg_texture = load_image_as_texture("media/sprites/clouds.png");

    game->projection_scale = 1;
    game->projection_rotation = 0;

    game->draw_grid = 0;

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
    GLdouble color[4] = {0, 0, 0, 1};
    Graphics_draw_rect(graphics, gfx_rect, color, game->bg_texture, 0);

    Graphics_scale_projection_matrix(graphics, game->projection_scale);
    glRotatef(game->projection_rotation, 0, 0, -1);

    // Draw the stuff
    LIST_FOREACH(game->entities, first, next, current) {
        GameEntity *thing = current->value;
        if (thing == NULL) break;
        GameEntity_render(thing, engine);
    }

    // Draw the grid
    if (!game->draw_grid || !game->world) return;
    Scene_draw_debug_grid(game, graphics);
}

World *Scene_create_world(Scene *scene, Physics *physics) {
    check_mem(scene);
    check_mem(physics);

    World *world = NEW(World, "The world");
    check_mem(world);

    int i = 0;
    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *entity = current->value;
        if (entity == NULL) break;

        Fixture *fixture = World_create_fixture(world);
        fixture->width =  1;
        fixture->height = fixture->width;// + fixture->width * 2 * i / NUM_BOXES;
        fixture->center.x = (world->width - (NUM_BOXES - i) * fixture->width * 4 + 2 * (NUM_BOXES + 1) * fixture->width) / 2;
        fixture->center.y = 1;
        fixture->time_scale = 1;
        fixture->rotation_radians = M_PI / 16 * (i % 8);
        Fixture_set_mass(fixture, 10);

        entity->fixture = fixture;
        /*
        PhysBox real_box = Fixture_real_box(fixture);
        fixture->history[0] = real_box;
        fixture->history[1] = real_box;
        */

        WorldGrid_add_fixture(world->grid, fixture);

        i++;
    }

    scene->world = world;
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

    if (input->debug_scene_draw_grid) scene->draw_grid = !(scene->draw_grid);

    double volume = scene->projection_scale * 128.f;
    Mix_VolumeMusic(volume);

    LIST_FOREACH(scene->entities, first, next, current) {
        GameEntity *entity = current->value;
        GameEntity_control(entity, input);
    }
}

void Scene_draw_debug_grid(Scene *scene, Graphics *graphics) {
    glDisable(GL_MULTISAMPLE);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(-SCREEN_WIDTH/2, -SCREEN_HEIGHT/2, 0.f);

    Graphics_scale_projection_matrix(graphics, scene->projection_scale);
    glRotatef(scene->projection_rotation, 0, 0, -1);
    glLineWidth(0);
    glColor3f(1.0, 0.0, 0.0);
    double grid = scene->world->grid_size;
    double ppm = scene->world->pixels_per_meter;
    int rows = ceil(scene->world->height / scene->world->grid_size);
    int cols = ceil(scene->world->width / scene->world->grid_size);
    int row = 0;
    int col = 0;
    for (row = 0; row < rows; row++) {
        for (col = 0; col < cols; col++) {
            glBegin(GL_LINES);
            glVertex2f((col * grid) * ppm, (row * grid) * ppm);
            glVertex2f((col * grid + grid) * ppm, (row * grid) * ppm);
            glEnd();

            glBegin(GL_LINES);
            glVertex2f((col * grid + grid) * ppm, (row * grid) * ppm);
            glVertex2f((col * grid + grid) * ppm, (row * grid + grid) * ppm);
            glEnd();

            glBegin(GL_LINES);
            glVertex2f((col * grid + grid) * ppm, (row * grid + grid) * ppm);
            glVertex2f((col * grid) * ppm, (row * grid + grid) * ppm);
            glEnd();

            glBegin(GL_LINES);
            glVertex2f((col * grid) * ppm, (row * grid + grid) * ppm);
            glVertex2f((col * grid) * ppm, (row * grid) * ppm);
            glEnd();
        }
    }
    glEnable(GL_MULTISAMPLE);
}

Object SceneProto = {
   .init = Scene_init,
   .destroy = Scene_destroy
};

