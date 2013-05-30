#ifndef __controller_h
#define __controller_h
#include "../prefix.h"

typedef enum {
    CONTROLLER_DPAD_NONE = 0,
    CONTROLLER_DPAD_UP = 1<<0,
    CONTROLLER_DPAD_RIGHT = 1<<1,
    CONTROLLER_DPAD_DOWN = 1<<2,
    CONTROLLER_DPAD_LEFT = 1<<3
} Controller_dpad_direction;

typedef struct Controller {
    Controller_dpad_direction dpad;
    int jump;
} Controller;

Controller *Controller_create();
void Controller_destroy(Controller *controller);
void Controller_reset(Controller *controller);

extern Object ControllerProto;
#endif
