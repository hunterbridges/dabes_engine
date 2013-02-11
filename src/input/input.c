#include "input.h"

int Input_init(void *self) {
    Input *input = self;
    int i = 0;
    for (i = 0; i < 4; i++) {
        input->controllers[i] = NEW(Controller, "A Controller");
    }
    Input_reset(input);
}

void Input_destroy(void *self) {
    Input *input = self;
    int i = 0;
    for (i = 0; i < 4; i++) {
        Controller *controller = input->controllers[i];
        controller->_(destroy)(controller);
    }
    free(input);
}

void Input_poll(Input *input) {
    // Hold keys
    Uint8 *keystate = SDL_GetKeyState(NULL);
    input->cam_zoom = 0;
    if (keystate[SDLK_j]) input->cam_zoom += 1;
    if (keystate[SDLK_k]) input->cam_zoom -= 1;

    input->cam_rotate = 0;
    if (keystate[SDLK_u]) input->cam_rotate += 1;
    if (keystate[SDLK_i]) input->cam_rotate -= 1;

    input->controllers[0]->dpad = CONTROLLER_DPAD_NONE;
    if (keystate[SDLK_UP])
        input->controllers[0]->dpad |= CONTROLLER_DPAD_UP;
    if (keystate[SDLK_DOWN])
        input->controllers[0]->dpad |= CONTROLLER_DPAD_DOWN;
    if (keystate[SDLK_LEFT])
        input->controllers[0]->dpad |= CONTROLLER_DPAD_LEFT;
    if (keystate[SDLK_RIGHT])
        input->controllers[0]->dpad |= CONTROLLER_DPAD_RIGHT;

    // Momentary keys
    SDL_Event event = {};
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_q) input->game_quit = 1;
            if (event.key.keysym.sym == SDLK_r) input->cam_reset = 1;

            if (event.key.keysym.sym == SDLK_SPACE)
                input->controllers[0]->jump = 1;
        }
    }
}

void Input_reset(Input *input) {
    check_mem(input);
    input->game_quit = 0;
    input->cam_reset = 0;
    input->cam_zoom = 0;
    input->cam_rotate = 0;
    int i = 0;
    for (i = 0; i < 4; i++) {
        Controller *controller = input->controllers[i];
        Controller_reset(controller);
    }
    return;
error:
    return;
}

Object InputProto = {
    .init = Input_init,
    .destroy = Input_destroy
};
