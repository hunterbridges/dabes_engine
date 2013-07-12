#include "chipmunk_recorder.h"
#include "../entities/entity.h"

void *ChipmunkRecorder_capture_frame(Recorder *recorder, size_t *size) {
    check(recorder != NULL, "No recorder to capture");
    check(recorder->entity != NULL, "No entity to capture recording of");

    ChipmunkRecorderFrame *frame = calloc(1, sizeof(ChipmunkRecorderFrame));
    *size = sizeof(ChipmunkRecorderFrame);
    check(frame != NULL, "Couldn't create frame");

    // is this a keyframe?
    ChipmunkRecorderCtx *context = (ChipmunkRecorderCtx *)recorder->context;
    short int is_keyframe =
        recorder->current_frame % context->keyframe_every == 0;
    frame->keyframe = is_keyframe;

    Body *body = recorder->entity->body;
    Sprite *sprite = recorder->entity->sprite;
    if (is_keyframe) {
        frame->pos = body->_(get_pos)(body);
        frame->velo = body->_(get_velo)(body);
        frame->sprite_frame = sprite->current_frame;
        frame->sprite_direction = sprite->direction;
        memcpy(&context->tracking_frame, frame, sizeof(ChipmunkRecorderFrame));
    } else {
        // Track the delta

        VPoint posnow = body->_(get_pos)(body);
        VPointRel posrel = VPoint_rel(posnow, context->tracking_frame.pos);
        if (posrel != VPointRelWithin) {
            frame->has_delta_pos = 1;
            frame->pos = VPoint_subtract(posnow, context->tracking_frame.pos);
            context->tracking_frame.pos = posnow;
        }

        VPoint velonow = body->_(get_velo)(body);
        VPointRel velorel = VPoint_rel(velonow, context->tracking_frame.velo);
        if (velorel != VPointRelWithin) {
            frame->has_delta_velo = 1;
            frame->velo =
                VPoint_subtract(velonow, context->tracking_frame.velo);
            context->tracking_frame.velo = velonow;
        }

        if (sprite->current_frame != context->tracking_frame.sprite_frame) {
            frame->has_sprite_frame = 1;
            frame->sprite_frame = sprite->current_frame;
            context->tracking_frame.sprite_frame = sprite->current_frame;
        }

        if (sprite->direction != context->tracking_frame.sprite_direction) {
            frame->has_sprite_direction = 1;
            frame->sprite_direction = sprite->direction;
            context->tracking_frame.sprite_direction = sprite->direction;
        }
    }

    context->prev_frame = frame;
    return frame;
error:
    return NULL;
}

void ChipmunkRecorder_apply_frame(struct Recorder *recorder, void *frame) {
    check(recorder != NULL, "No recorder to apply");
    check(frame != NULL, "No recorder to apply");
    check(recorder->entity != NULL, "No entity to apply recording to");

    Body *body = recorder->entity->body;
    Sprite *sprite = recorder->entity->sprite;

    ChipmunkRecorderCtx *context = (ChipmunkRecorderCtx *)recorder->context;
    ChipmunkRecorderFrame *f = (ChipmunkRecorderFrame *)frame;
    if (f->keyframe) {
        body->_(set_pos)(body, f->pos);
        body->_(set_velo)(body, f->velo);
        sprite->current_frame = f->sprite_frame;
        sprite->direction = f->sprite_direction;
    } else {
        if (f->has_delta_pos) {
            VPoint pos = body->_(get_pos)(body);
            pos = VPoint_add(pos, f->pos);
            body->_(set_pos)(body, pos);
        }
        if (f->has_delta_velo) {
            VPoint velo = body->_(get_velo)(body);
            velo = VPoint_add(velo, f->velo);
            body->_(set_velo)(body, velo);
        }
        if (f->has_sprite_frame) {
            sprite->current_frame = f->sprite_frame;
        }
        if (f->has_sprite_direction) {
            sprite->direction = f->sprite_direction;
        }
    }
    context->prev_frame = f;

    return;
error:
    return;
}

void ChipmunkRecorder_clear_frames(Recorder *recorder) {
    check(recorder != NULL, "No recorder to clear");

    // Destroy frames
    DArray_clear_destroy(recorder->frames);
    recorder->frames = DArray_create(sizeof(ChipmunkRecorderFrame), 60 * 5);

    recorder->num_frames = 0;
    recorder->avg_frame_size = 0;
    recorder->total_frame_size = 0;

    return;
error:
    return;
}

void ChipmunkRecorder_rewind(struct Recorder *recorder) {
    check(recorder != NULL, "No recorder to reset");
    recorder->current_frame = 0;

    ChipmunkRecorderCtx *context = (ChipmunkRecorderCtx *)recorder->context;
    context->prev_frame = NULL;

    return;
error:
    return;
}

void ChipmunkRecorder_start_play_cb(struct Recorder *recorder) {
    recorder->entity->sprite->manual_frames = 1;
}

void ChipmunkRecorder_stop_play_cb(struct Recorder *recorder) {
    recorder->entity->sprite->manual_frames = 0;
}

RecorderProto ChipmunkRecorderProto = {
    .capture_frame = ChipmunkRecorder_capture_frame,
    .apply_frame = ChipmunkRecorder_apply_frame,
    .clear_frames = ChipmunkRecorder_clear_frames,
    .rewind = ChipmunkRecorder_rewind,
    .start_play_cb = ChipmunkRecorder_start_play_cb,
    .stop_play_cb = ChipmunkRecorder_stop_play_cb
};

Recorder *ChipmunkRecorder_create(int preroll_ms, int fps) {
    Recorder *recorder =
        Recorder_create(ChipmunkRecorderProto, preroll_ms, fps);
    check(recorder != NULL, "Couldn't create chipmunk recorder");

    recorder->context = calloc(1, sizeof(ChipmunkRecorderCtx));
    check(recorder->context != NULL, "Couldn't create recorder context");

    ChipmunkRecorderCtx *context = recorder->context;
    context->keyframe_every = 5;

    return recorder;
error:
    if (recorder) Recorder_destroy(recorder);
    return NULL;
}
