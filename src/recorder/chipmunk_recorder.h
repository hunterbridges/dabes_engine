#ifndef __chipmunk_recorder_h
#define __chipmunk_recorder_h
#include <chipmunk/chipmunk.h>
#include "recorder.h"
#include "../graphics/sprite.h"

typedef struct ChipmunkRecorderFrame {
    dab_uint8 keyframe;

    dab_uint8 has_delta_pos;
    VPoint pos;

    dab_uint8 has_delta_velo;
    VPoint velo;

    dab_uint8 has_sprite_frame;
    dab_uint16 sprite_frame;

    dab_uint8 has_sprite_direction;
    SpriteDirection sprite_direction;
} ChipmunkRecorderFrame;

typedef struct ChipmunkRecorderCtx {
    dab_int keyframe_every;
    ChipmunkRecorderFrame *prev_frame;
    ChipmunkRecorderFrame tracking_frame;
    int was_rogue;
    int was_manual_frames;
} ChipmunkRecorderCtx;

extern RecorderProto ChipmunkRecorderProto;

Recorder *ChipmunkRecorder_create(int preroll_ms, int fps);
int ChipmunkRecorder_contextualize(Recorder *recorder);

#endif
