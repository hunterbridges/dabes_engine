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
    Object proto;
    Controller_dpad_direction dpad;
    int jump;
} Controller;

int Controller_init(void *self);
void Controller_destroy(void *self);
void Controller_reset(Controller *controller);

extern Object ControllerProto;
#endif
