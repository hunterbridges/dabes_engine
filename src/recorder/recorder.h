#ifndef __recorder_h
#define __recorder_h
#include <lcthw/darray.h>
#include "../entities/entity.h"

struct Recorder;
typedef struct RecorderProto {
    void *(*capture_frame)(struct Recorder *recorder, size_t *size);
    void (*apply_frame)(struct Recorder *recorder, void *frame);
    void (*clear_frames)(struct Recorder *recorder);
    void (*rewind)(struct Recorder *recorder);
    void (*start_play_cb)(struct Recorder *recorder);
    void (*stop_play_cb)(struct Recorder *recorder);
} RecorderProto;

typedef enum {
    RecorderStateIdle = 0,
    RecorderStateRecording = 1,
    RecorderStatePlaying = 2
} RecorderState;

typedef struct Recorder {
    RecorderProto proto;
    Entity *entity;
    DArray *frames;

    void *context;

    int current_frame;
    RecorderState state;

    int num_frames;
    double avg_frame_size;
    size_t total_frame_size;
} Recorder;

Recorder *Recorder_create(RecorderProto proto, int preroll_ms, int fps);
void Recorder_destroy(Recorder *recorder);
void Recorder_write_frame(Recorder *recorder, void *frame, size_t size);
void *Recorder_read_frame(Recorder *recorder);
void Recorder_set_state(Recorder *recorder, RecorderState state);

#endif
