#include "controller.h"

Controller *Controller_create() {
    Controller *controller = calloc(1, sizeof(Controller));
    check(controller != NULL, "Couldn't create Controller");
    Controller_reset(controller);
    return controller;
error:
    return NULL;
}

void Controller_destroy(Controller *controller) {
    check(controller != NULL, "No Controller to destroy");
    free(controller);
    return;
error:
    return;
}

void Controller_reset(Controller *controller) {
    check_mem(controller);
    controller->dpad = CONTROLLER_DPAD_NONE;
    controller->jump = 0;

    if (controller->touch_state & CONTROLLER_TOUCH_MOVED) {
        controller->touch_state &= ~(CONTROLLER_TOUCH_MOVED);
    }

    if (controller->touch_state & CONTROLLER_TOUCH_HOLD_CHANGED) {
        controller->touch_state &= ~(CONTROLLER_TOUCH_HOLD_CHANGED);
    }

    return;
error:
    return;
}
