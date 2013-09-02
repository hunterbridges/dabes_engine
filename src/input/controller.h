#ifndef __controller_h
#define __controller_h
#include "../prefix.h"
#include "../math/vpoint.h"

typedef enum {
    CONTROLLER_DPAD_NONE = 0,
    CONTROLLER_DPAD_UP = 1<<0,
    CONTROLLER_DPAD_RIGHT = 1<<1,
    CONTROLLER_DPAD_DOWN = 1<<2,
    CONTROLLER_DPAD_LEFT = 1<<3
} Controller_dpad_direction;

typedef enum {
    CONTROLLER_TOUCH_NONE = 0,
    CONTROLLER_TOUCH_HOLD = 1,
    CONTROLLER_TOUCH_HOLD_CHANGED = 1 << 1,
    CONTROLLER_TOUCH_MOVED = 1 << 2
} ControllerTouchState;

typedef struct Controller {
    Controller_dpad_direction dpad;
    int jump;

    ControllerTouchState touch_state;
    VPoint touch_pos;
} Controller;

Controller *Controller_create();
void Controller_destroy(Controller *controller);
void Controller_reset(Controller *controller);

extern Object ControllerProto;

#endif
