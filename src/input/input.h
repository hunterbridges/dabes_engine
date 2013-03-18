#ifndef __input_h
#define __input_h
#include "../prefix.h"
#include "controller.h"

typedef struct Input {
    Object proto;
    Controller *controllers[4];
    int game_quit;
    int debug_scene_draw_grid;
    int cam_reset;
    int cam_zoom;
    int cam_rotate;
} Input;

extern Object InputProto;

int Input_init(void *self);
void Input_destroy(void *self);
void Input_poll(Input *input);
void Input_reset(Input *input);

#endif
