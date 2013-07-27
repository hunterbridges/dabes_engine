#ifndef __input_h
#define __input_h
#include "../prefix.h"
#include "controller.h"
#include "../math/vpoint.h"

#define INPUT_NUM_CONTROLLERS 4

typedef struct Input {
    Controller *controllers[INPUT_NUM_CONTROLLERS];
    int game_quit;
    int debug_scene_draw_grid;
    int cam_reset;
    int cam_zoom;
    int cam_rotate;
    VPoint cam_focal_pan;
    VPoint cam_translate_pan;
    int cam_debug;
    int phys_render;
  
} Input;

Input *Input_create();
void Input_destroy(Input *input);
void Input_poll(Input *input);
void Input_touch(Input *input, Input *touch_input);
void Input_reset(Input *input);

#endif
