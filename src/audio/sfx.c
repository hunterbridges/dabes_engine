#include "sfx.h"

Sfx *Sfx_load(char *filename) {
    Sfx *sfx = malloc(sizeof(Sfx));
    check(sfx != NULL, "Couldn't create SFX");
    sfx->volume = 1;
    sfx->playing = 0;
    sfx->ended = 0;

    alGenSources(1, &sfx->source);
    Audio_check();

    alSource3f(sfx->source, AL_POSITION, 0.0, 0.0, 0.0);
    alSource3f(sfx->source, AL_VELOCITY, 0.0, 0.0, 0.0);
    alSource3f(sfx->source, AL_DIRECTION, 0.0, 0.0, 0.0);
    alSourcef(sfx->source, AL_ROLLOFF_FACTOR, 0.0);
    alSourcei(sfx->source, AL_SOURCE_RELATIVE, AL_TRUE);

    sfx->ogg_stream = OggStream_create(filename, sfx->source);

    return sfx;
error:
    return NULL;
}

void Sfx_destroy(Sfx *sfx) {
    check (sfx != NULL, "No SFX to destroy");
    OggStream_stop(sfx->ogg_stream);
    OggStream_destroy(sfx->ogg_stream);
    alDeleteSources(1, &sfx->source);
    free(sfx);
    return;
error:
    return;
}

void Sfx_play(Sfx *sfx) {
    if (sfx == NULL) return;
    if (sfx->playing) return;
    sfx->playing = 1;
    OggStream_play(sfx->ogg_stream);
}

void Sfx_update(Sfx *sfx) {
    OggStream_update(sfx->ogg_stream);
    if (sfx->ogg_stream && sfx->ogg_stream->ended) sfx->ended = 1;
}

void Sfx_set_volume(Sfx *sfx, double volume) {
    sfx->volume = volume;
    alSourcef(sfx->source, AL_GAIN, volume);
}
