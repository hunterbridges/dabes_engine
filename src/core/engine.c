#include "engine.h"

int Engine_init(void *self) {
    check_mem(self);
    Engine *engine = self;

    engine->audio = NEW(Audio, "Audio Engine");
    engine->input = NEW(Input, "Input Engine");
    engine->graphics = NEW(Graphics, "Graphics Engine");
    engine->physics = NEW(Physics, "Physics Engine");

    return 1;
error:
    return 0;
}

void Engine_destroy(void *self) {
    check_mem(self);
    Engine *engine = self;

    engine->audio->_(destroy)(engine->audio);
    engine->input->_(destroy)(engine->input);
    engine->graphics->_(destroy)(engine->graphics);
    engine->physics->_(destroy)(engine->physics);

    free(self);
    return;
error:
    free(self);
}

Object EngineProto = {
    .init = Engine_init,
    .destroy = Engine_destroy
};
