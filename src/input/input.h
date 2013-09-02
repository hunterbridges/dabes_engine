#ifndef __input_h
#define __input_h
#include "../prefix.h"
#include "controller.h"
#include "../math/vpoint.h"

#define INPUT_NUM_CONTROLLERS 4

typedef enum InputStyle {
    INPUT_STYLE_LEFT_RIGHT = 0,
    INPUT_STYLE_TOUCHPAD,
    INPUT_STYLE_USER0 = 16,
    INPUT_STYLE_USER1,
    INPUT_STYLE_USER2,
    INPUT_STYLE_USER3,
    INPUT_STYLE_USER4,
    INPUT_STYLE_USER5,
    INPUT_STYLE_USER6,
    INPUT_STYLE_USER7,
    INPUT_STYLE_USER8
} InputStyle;

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
    
    InputStyle preferred_style;
    void (*change_preferred_style_cb)(struct Input *input,
                                      InputStyle old_style,
                                      InputStyle new_style);
} Input;

Input *Input_create();
void Input_destroy(Input *input);
void Input_poll(Input *input);
void Input_touch(Input *input, Input *touch_input);
void Input_reset(Input *input);
void Input_change_preferred_style(Input *input, InputStyle preferred);

#endif
