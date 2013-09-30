#include "recorder.h"

RecorderProto RecorderNullProto = {
    .capture_frame = NULL,
    .apply_frame = NULL,
    .clear_frames = NULL,
    .rewind = NULL,
    .start_play_cb = NULL,
    .stop_play_cb = NULL,
    .pack = NULL,
    .unpack = NULL
};

Recorder *Recorder_create(int preroll_ms, int fps) {
    Recorder *recorder = calloc(1, sizeof(Recorder));
    check(recorder != NULL, "Couldn't create Recorder");
    recorder->proto = RecorderNullProto;
    // TODO: This Max thing is weird. Figure out what to do about this param
    // when instantiating in Lua from C.
    recorder->frames = DArray_create(sizeof(void *), MAX(preroll_ms, 5) * fps);

    // Generate id
    int strlen = sizeof(Recorder *) * 2 + 2;
    char *id = calloc(1, sizeof(char) * (strlen + 1));
    sprintf(id, "%p", recorder);
    recorder->id = id;

    return recorder;
error:
    return NULL;
}

void Recorder_destroy(Recorder *recorder) {
    check(recorder != NULL, "No recorder to destroy");

    recorder->_(clear_frames)(recorder);
    if (recorder->context) free(recorder->context);
    DArray_destroy(recorder->frames);
    free(recorder->id);
    free(recorder);

    return;
error:
    return;
}

void Recorder_write_frame(Recorder *recorder, void *frame, size_t size) {
    check(recorder != NULL, "No recorder to write");
    check(recorder->state == RecorderStateRecording,
            "Recorder is not recording");

    DArray_push(recorder->frames, frame);

    // Track statistics about our frames.
    if (recorder->num_frames == 0) {
        recorder->avg_frame_size = (double)size;
    } else {
        recorder->avg_frame_size =
            (recorder->avg_frame_size * recorder->num_frames + (double)size) /
                (recorder->num_frames + 1);
    }

    recorder->total_frame_size += size;
    recorder->num_frames = DArray_count(recorder->frames);
    recorder->current_frame = recorder->num_frames - 1;

    return;
error:
    return;
}

void *Recorder_read_frame(Recorder *recorder) {
    check(recorder != NULL, "No recorder to read");
    check(recorder->state == RecorderStatePlaying, "Recorder is not playing");

    if (recorder->current_frame >= recorder->num_frames) return NULL;
    void *frame = DArray_get(recorder->frames, recorder->current_frame);

    recorder->current_frame++;

    return frame;
error:
    return NULL;
}

void Recorder_set_state(Recorder *recorder, RecorderState state) {
    check(recorder != NULL, "No recorder to set state of");
    if (recorder->state == state) return;

    if (recorder->state == RecorderStatePlaying) {
        recorder->_(stop_play_cb)(recorder);
    }
    
    if (recorder->state == RecorderStateRecording) {
        recorder->state = state;
        recorder->_(rewind)(recorder);
    } else {
        recorder->_(rewind)(recorder);
        recorder->state = state;
    }
    
    if (state == RecorderStateRecording) {
        recorder->_(clear_frames)(recorder);
    }
    if (state == RecorderStatePlaying) {
        recorder->_(start_play_cb)(recorder);
    }
    return;
error:
    return;
}
