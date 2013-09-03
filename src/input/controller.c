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

void Controller_reset_touches(Controller *controller) {
    check(controller != NULL, "Controller required");
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

void Controller_reset(Controller *controller) {
    check_mem(controller);
    controller->dpad = CONTROLLER_DPAD_NONE;
    controller->jump = 0;

    Controller_reset_touches(controller);
    return;
error:
    return;
}

void Controller_debug_touch_state(Controller *controller, const char *msg) {
    check(controller != NULL, "Controller required");
    log_info("%s - DOWN %d, CHANGED %d, MOVED %i, POS <%.02f, %.02f>", msg,
             !!(controller->touch_state & CONTROLLER_TOUCH_HOLD),
             !!(controller->touch_state & CONTROLLER_TOUCH_HOLD_CHANGED),
             !!(controller->touch_state & CONTROLLER_TOUCH_MOVED),
             controller->touch_pos.x, controller->touch_pos.y);
    return;
error:
    return;
}
