#ifndef __ogg_stream_h
#define __ogg_stream_h
#include <stdio.h>
#include <stdlib.h>

#if defined(DABES_IOS) || defined(DABES_MAC)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#include <Ogg/ogg.h>
#include <Vorbis/codec.h>
#include <Vorbis/vorbisenc.h>
#include <Vorbis/vorbisfile.h>
#else
#include <al/al.h>
#include <al/alc.h>
#include <ogg/ogg.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/vorbisfile.h>
#endif

#include "../prefix.h"
#include "audio.h"

#define OGG_BUFFER_COUNT 3

typedef struct OggStream {
    char *filename;
    FILE *file;
    OggVorbis_File stream;
    vorbis_info *vorbis_info;
    vorbis_comment *vorbis_comment;

    int buf_count;
    ALuint buffers[OGG_BUFFER_COUNT];
    ALuint source;
    ALenum format;

    int started;
    int should_loop;
    int eof;
    int ended;
} OggStream;

OggStream *OggStream_create(char *file, ALuint source);
OggStream *OggStream_clone(OggStream *ogg_stream);
void OggStream_destroy(OggStream *ogg_stream);
void OggStream_debug(OggStream *ogg_stream);
int OggStream_stop(OggStream *ogg_stream);
int OggStream_play(OggStream *ogg_stream);
int OggStream_playing(OggStream *ogg_stream);
int OggStream_update(OggStream *ogg_stream);
void OggStream_rewind(OggStream *ogg_stream);

#endif
