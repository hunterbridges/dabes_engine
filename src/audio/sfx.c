#include "sfx.h"

void Sfx_t_initialize(Sfx *sfx);
void Sfx_t_uninitialize(Sfx *sfx);
void Sfx_t_end(Sfx *sfx);

#pragma mark - Main Thread

Sfx *Sfx_load(char *filename) {
    Sfx *sfx = calloc(1, sizeof(Sfx));
    check(sfx != NULL, "Couldn't create SFX");
    sfx->volume = 1;

    FILE *fcheck = fopen(filename, "r");
    check(fcheck != NULL, "Failed to find %s", filename);
    fclose(fcheck);
    sfx->filename = calloc(1, sizeof(char) * (strlen(filename) + 1));
    strcpy(sfx->filename, filename);

    return sfx;
error:
    if (sfx) Sfx_destroy(sfx);
    return NULL;
}

void Sfx_destroy(Sfx *sfx) {
    check (sfx != NULL, "No SFX to destroy");
    if (!sfx->ended) {
        // TODO: Block condvar and wait for end signal
        Sfx_t_end(sfx);
    }
    assert(sfx->initialized == 0);
    free(sfx->filename);
    free(sfx);
    return;
error:
    return;
}

void Sfx_play(Sfx *sfx) {
    if (sfx == NULL) return;
    if (sfx->playing) return;
    sfx->_needs_play = 1;
}

void Sfx_set_volume(Sfx *sfx, double volume) {
    sfx->volume = volume;

    if (sfx->initialized) {
        // TODO: Lock? This might be thread safe
        alSourcef(sfx->source, AL_GAIN, volume);
    }
}

#pragma mark - Audio Thread

void Sfx_t_end(Sfx *sfx) {
    alSourceStop(sfx->source);
    Sfx_t_uninitialize(sfx);
    sfx->ended = 1;
}

void Sfx_t_update(Sfx *sfx) {
    if (sfx->ended) return;

    // TODO: Lock
    if (!sfx->initialized) {
        Sfx_t_initialize(sfx);
    }
    if (sfx->_needs_play) {
        sfx->_needs_play = 0;
        sfx->playing = 1;
        OggStream_play(sfx->ogg_stream);
    }
    OggStream_update(sfx->ogg_stream);
    ALenum state;
    alGetSourcei(sfx->source, AL_SOURCE_STATE, &state);
    if (sfx->ogg_stream && sfx->ogg_stream->ended && state == AL_STOPPED) {
        Sfx_t_end(sfx);
    }
}

void Sfx_t_initialize(Sfx *sfx) {
    assert(sfx->initialized == 0);
    alGenSources(1, &sfx->source);
    Audio_check();

    alSource3f(sfx->source, AL_POSITION, 0.0, 0.0, 0.0);
    alSource3f(sfx->source, AL_VELOCITY, 0.0, 0.0, 0.0);
    alSource3f(sfx->source, AL_DIRECTION, 0.0, 0.0, 0.0);
    alSourcef(sfx->source, AL_ROLLOFF_FACTOR, 0.0);
    alSourcei(sfx->source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcef(sfx->source, AL_GAIN, sfx->volume);

    sfx->ogg_stream = OggStream_create(sfx->filename, sfx->source);
    check(sfx->ogg_stream != NULL, "Couldn't create Sfx OGG stream");

    sfx->initialized = 1;
    return;
error:
    return;
}

void Sfx_t_uninitialize(Sfx *sfx) {
    assert(sfx->initialized == 1);

    OggStream_stop(sfx->ogg_stream);
    OggStream_destroy(sfx->ogg_stream);
    sfx->ogg_stream = NULL;

    alDeleteSources(1, &sfx->source);
    sfx->source = 0;
    sfx->initialized = 0;
}

