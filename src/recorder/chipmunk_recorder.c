#include "chipmunk_recorder.h"
#include "../entities/entity.h"

static const int CHIPMUNK_RECORDER_DEFAULT_KEYFRAME_FREQ = 120;

void *ChipmunkRecorder_capture_frame(Recorder *recorder, size_t *size) {
    check(recorder != NULL, "No recorder to capture");
    check(recorder->entity != NULL, "No entity to capture recording of");

    ChipmunkRecorderFrame *frame = calloc(1, sizeof(ChipmunkRecorderFrame));
    check(frame != NULL, "Couldn't create frame");

    // is this a keyframe?
    ChipmunkRecorderCtx *context = (ChipmunkRecorderCtx *)recorder->context;
    short int is_keyframe =
        (recorder->current_frame % context->keyframe_every) == 0;
    if (recorder->entity->force_keyframe) {
        is_keyframe = 1;
        recorder->entity->force_keyframe = 0;
    }

    frame->keyframe = is_keyframe;

    Body *body = recorder->entity->body;
    Sprite *sprite = recorder->entity->sprite;
    if (is_keyframe) {
        frame->pos = body->_(get_pos)(body);
        frame->velo = body->_(get_velo)(body);
        frame->sprite_frame = sprite->current_frame;
        frame->sprite_direction = sprite->direction;
        memcpy(&context->tracking_frame, frame, sizeof(ChipmunkRecorderFrame));
        *size = sizeof(ChipmunkRecorderFrame);
    } else {
        size_t f_size = sizeof(unsigned char);

        VPoint posnow = body->_(get_pos)(body);
        VPointRel posrel = VPoint_rel(posnow, context->tracking_frame.pos);
        if (posrel != VPointRelWithin) {
            frame->has_delta_pos = 1;
            frame->pos = VPoint_subtract(posnow, context->tracking_frame.pos);
            context->tracking_frame.pos = posnow;
            f_size += sizeof(VPoint);
        }

        VPoint velonow = body->_(get_velo)(body);
        VPointRel velorel = VPoint_rel(velonow, context->tracking_frame.velo);
        if (velorel != VPointRelWithin) {
            frame->has_delta_velo = 1;
            frame->velo =
                VPoint_subtract(velonow, context->tracking_frame.velo);
            context->tracking_frame.velo = velonow;
            f_size += sizeof(VPoint);
        }

        if (sprite->current_frame != context->tracking_frame.sprite_frame) {
            frame->has_sprite_frame = 1;
            frame->sprite_frame = sprite->current_frame;
            context->tracking_frame.sprite_frame = sprite->current_frame;
            f_size += sizeof(int);
        }

        if (sprite->direction != context->tracking_frame.sprite_direction) {
            frame->has_sprite_direction = 1;
            frame->sprite_direction = sprite->direction;
            context->tracking_frame.sprite_direction = sprite->direction;
            f_size += sizeof(int);
        }

        *size = f_size;
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
    if (recorder->frames) {
        DArray_clear_destroy(recorder->frames);
    }

    recorder->frames = DArray_create(sizeof(ChipmunkRecorderFrame), FPS * 5);

    recorder->num_frames = 0;
    recorder->avg_frame_size = 0;
    recorder->total_frame_size = 0;

    return;
error:
    return;
}

void ChipmunkRecorder_rewind(struct Recorder *recorder) {
    check(recorder != NULL, "No recorder to reset");
    check(recorder->state != RecorderStateRecording,
            "Can't rewind while recording");
    recorder->current_frame = 0;

    ChipmunkRecorderCtx *context = (ChipmunkRecorderCtx *)recorder->context;
    context->prev_frame = NULL;

    return;
error:
    return;
}

void ChipmunkRecorder_start_play_cb(struct Recorder *recorder) {
    recorder->entity->sprite->manual_frames = 1;
    recorder->entity->auto_control = 1;
    Body *body = recorder->entity->body;
    body->_(set_is_rogue)(body, 1);
}

void ChipmunkRecorder_stop_play_cb(struct Recorder *recorder) {
    recorder->entity->sprite->manual_frames = 0;
    recorder->entity->auto_control = 0;
    Body *body = recorder->entity->body;
    body->_(set_is_rogue)(body, 0);
}

void ChipmunkRecorder_pack(struct Recorder *recorder, dab_uchar **buffer,
                           size_t *size) {
    check(recorder != NULL, "No recorder to pack");

    size_t bufcap = sizeof(ChipmunkRecorderCtx) +
        recorder->num_frames * sizeof(ChipmunkRecorderFrame);
    dab_uchar *buf = malloc(bufcap);
    check(buf != NULL, "Unable to create recorder pack buffer");

    dab_uchar *run = buf;
    size_t bufsize = 0;

    // First thing is copy over the context, this is our buffer header.
    memcpy(run, recorder->context, sizeof(ChipmunkRecorderCtx));
    run += sizeof(ChipmunkRecorderCtx);
    bufsize += sizeof(ChipmunkRecorderCtx);

    int i = 0;
    for (i = 0; i < recorder->num_frames && bufsize < bufcap; i++) {
        ChipmunkRecorderFrame *frame = DArray_get(recorder->frames, i);

        dab_uint8 flags = 0;

        // Flags
        // 00000000
        // ^ ( << 7 ) keyframe
        //  ^ ( << 6 ) has_delta_pos
        //   ^ ( << 5 ) has_delta_velo
        //    ^ ( << 4 ) has_sprite_frame
        //     ^ ( << 3 ) has_sprite_direction
        flags ^= frame->keyframe << 7;
        flags ^= frame->has_delta_pos << 6;
        flags ^= frame->has_delta_velo << 5;
        flags ^= frame->has_sprite_frame << 4;
        flags ^= frame->has_sprite_direction << 3;

        *run = flags;
        run++;
        bufsize++;

        if (frame->keyframe || frame->has_delta_pos) {
            memcpy(run, &frame->pos, sizeof(VPoint));
            run += sizeof(VPoint);
            bufsize += sizeof(VPoint);
        }

        if (frame->keyframe || frame->has_delta_velo) {
            memcpy(run, &frame->velo, sizeof(VPoint));
            run += sizeof(VPoint);
            bufsize += sizeof(VPoint);
        }

        if (frame->keyframe || frame->has_sprite_frame) {
            memcpy(run, &frame->sprite_frame, sizeof(dab_uint16));
            run += sizeof(dab_uint16);
            bufsize += sizeof(dab_uint16);
        }

        if (frame->keyframe || frame->has_sprite_direction) {
            memcpy(run, &frame->sprite_direction, sizeof(SpriteDirection));
            run += sizeof(SpriteDirection);
            bufsize += sizeof(SpriteDirection);
        }
    }

    buf = realloc(buf, bufsize);

    *buffer = buf;
    *size = bufsize;

    return;
error:
    return;
}

void ChipmunkRecorder_unpack(struct Recorder *recorder, dab_uchar *buffer,
                             size_t size) {
    check(recorder != NULL, "No recorder to pack");
    check(buffer != NULL, "No buffer to unpack");
    check(size > 0, "Buffer can not be empty");
    recorder->_(clear_frames)(recorder);
    Recorder_set_state(recorder, RecorderStateRecording);

    size_t has_read = 0;

    // Read out the buffer header
    dab_uchar *run = buffer;
    memcpy(recorder->context, buffer, sizeof(ChipmunkRecorderCtx));

    // Might be a bogus pointer.
    ((ChipmunkRecorderCtx *)recorder->context)->prev_frame = NULL;

    run += sizeof(ChipmunkRecorderCtx);
    has_read += sizeof(ChipmunkRecorderCtx);

    while (has_read < size) {
        size_t start_size = has_read;
        ChipmunkRecorderFrame *frame = calloc(1, sizeof(ChipmunkRecorderFrame));

        // Read the flags
        dab_uint8 flags = *run;
        frame->keyframe             = (flags & 1 << 7) != 0;
        frame->has_delta_pos        = (flags & 1 << 6) != 0;
        frame->has_delta_velo       = (flags & 1 << 5) != 0;
        frame->has_sprite_frame     = (flags & 1 << 4) != 0;
        frame->has_sprite_direction = (flags & 1 << 3) != 0;
        run++;
        has_read++;

        if (frame->keyframe || frame->has_delta_pos) {
            memcpy(&frame->pos, run, sizeof(VPoint));
            run += sizeof(VPoint);
            has_read += sizeof(VPoint);
        }

        if (frame->keyframe || frame->has_delta_velo) {
            memcpy(&frame->velo, run, sizeof(VPoint));
            run += sizeof(VPoint);
            has_read += sizeof(VPoint);
        }

        if (frame->keyframe || frame->has_sprite_frame) {
            memcpy(&frame->sprite_frame, run, sizeof(dab_uint16));
            run += sizeof(dab_uint16);
            has_read += sizeof(dab_uint16);
        }

        if (frame->keyframe || frame->has_sprite_direction) {
            memcpy(&frame->sprite_direction, run, sizeof(SpriteDirection));
            run += sizeof(SpriteDirection);
            has_read += sizeof(SpriteDirection);
        }

        Recorder_write_frame(recorder, frame, has_read - start_size);
    }

    assert(has_read == size);
    Recorder_set_state(recorder, RecorderStateIdle);
    return;
error:
    return;
}

RecorderProto ChipmunkRecorderProto = {
    .capture_frame = ChipmunkRecorder_capture_frame,
    .apply_frame = ChipmunkRecorder_apply_frame,
    .clear_frames = ChipmunkRecorder_clear_frames,
    .rewind = ChipmunkRecorder_rewind,
    .start_play_cb = ChipmunkRecorder_start_play_cb,
    .stop_play_cb = ChipmunkRecorder_stop_play_cb,
    .pack = ChipmunkRecorder_pack,
    .unpack = ChipmunkRecorder_unpack
};

Recorder *ChipmunkRecorder_create(int preroll_ms, int fps) {
    Recorder *recorder = Recorder_create(preroll_ms, fps);
    check(recorder != NULL, "Couldn't create chipmunk recorder");

    int rc = ChipmunkRecorder_contextualize(recorder);
    check(rc == 1, "Couldn't Contextualize recorder");

    return recorder;
error:
    if (recorder) Recorder_destroy(recorder);
    return NULL;
}

int ChipmunkRecorder_contextualize(Recorder *recorder) {
    check(recorder != NULL, "Couldn't create chipmunk recorder");
    if (memcmp(&recorder->proto, &ChipmunkRecorderProto,
               sizeof(RecorderProto)) == 0) {
        return 0;
    }

    recorder->proto = ChipmunkRecorderProto;

    if (recorder->context) free(recorder->context);

    recorder->context = calloc(1, sizeof(ChipmunkRecorderCtx));
    check(recorder->context != NULL, "Couldn't create recorder context");

    ChipmunkRecorderCtx *context = recorder->context;
    context->keyframe_every = CHIPMUNK_RECORDER_DEFAULT_KEYFRAME_FREQ;

    return 1;
error:
    return 0;
}
