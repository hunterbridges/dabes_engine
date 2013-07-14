#include "recorder.h"

Recorder *Recorder_create(RecorderProto proto, int preroll_ms, int fps) {
    Recorder *recorder = calloc(1, sizeof(Recorder));
    check(recorder != NULL, "Couldn't create Recorder");
    recorder->proto = proto;
    recorder->frames = DArray_create(sizeof(void *), preroll_ms * fps);
    return recorder;
error:
    return NULL;
}

void Recorder_destroy(Recorder *recorder) {
    check(recorder != NULL, "No recorder to destroy");

    recorder->_(clear_frames)(recorder);
    if (recorder->context) free(recorder->context);
    DArray_destroy(recorder->frames);
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
        // This shouldn't mess anything up.
        unsigned char *buf = NULL;
        size_t sz = 0;
        recorder->_(pack)(recorder, &buf, &sz);
        recorder->_(unpack)(recorder, buf, sz);
        free(buf);
    }

    recorder->state = state;
    recorder->_(rewind)(recorder);

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
