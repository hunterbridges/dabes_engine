#include "controller.h"

int Controller_init(void *self) {
    Controller *controller = self;
    Controller_reset(controller);
    return 1;
}

void Controller_destroy(void *self) {
    Controller *controller = self;
    check_mem(controller);
    free(controller);
    return;
error:
    return;
}

void Controller_reset(Controller *controller) {
    check_mem(controller);
    controller->dpad = CONTROLLER_DPAD_NONE;
    controller->jump = 0;
    return;
error:
    return;
}

Object ControllerProto = {
    .init = Controller_init,
    .destroy = Controller_destroy
};
