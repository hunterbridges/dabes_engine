#ifndef __chipmunk_recorder_h
#define __chipmunk_recorder_h
#include <chipmunk/chipmunk.h>
#include "recorder.h"
#include "../graphics/sprite.h"

typedef struct ChipmunkRecorderFrame {
    short int keyframe;

    short int has_delta_pos;
    VPoint pos;

    short int has_delta_velo;
    VPoint velo;
  
    short int has_sprite_frame;
    int sprite_frame;

    short int has_sprite_direction;
    SpriteDirection sprite_direction;
} ChipmunkRecorderFrame;

typedef struct ChipmunkRecorderCtx {
    int keyframe_every;
    ChipmunkRecorderFrame *prev_frame;
    ChipmunkRecorderFrame tracking_frame;
} ChipmunkRecorderCtx;

extern RecorderProto ChipmunkRecorderProto;

Recorder *ChipmunkRecorder_create(int preroll_ms, int fps);

#endif
