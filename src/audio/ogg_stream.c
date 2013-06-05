#include "ogg_stream.h"

const int BUFFER_SIZE = 4096 * 8;

int OggStream_stream(OggStream *ogg_stream, ALuint buffer);
void OggStream_empty(OggStream *ogg_stream);
const char *OggStream_error_string(int code);

int OggStream_open_decoder(OggStream *ogg_stream) {
    check(ogg_stream != NULL, "No Ogg Stream to open decoder for");
    int result = ov_open(ogg_stream->file, &ogg_stream->stream, NULL, 0);
    check(result >= 0, "Couldn't open OGG stream: %s",
            OggStream_error_string(result));

    ogg_stream->vorbis_info = ov_info(&ogg_stream->stream, -1);
    ogg_stream->vorbis_comment = ov_comment(&ogg_stream->stream, -1);

    return 1;
error:
    return 0;
}

int OggStream_close_decoder(OggStream *ogg_stream) {
    check(ogg_stream != NULL, "No Ogg Stream to close decoder for");

    ov_clear(&ogg_stream->stream);
    memset(&ogg_stream->stream, 0, sizeof(OggVorbis_File));

    ogg_stream->vorbis_info = 0;
    ogg_stream->vorbis_comment = 0;
    return 1;
error:
    return 0;
}

OggStream *OggStream_create(char *file, ALuint source) {
    OggStream *ogg_stream = calloc(1, sizeof(OggStream));
    check(ogg_stream != NULL, "Couldn't create OGG Stream");

    ogg_stream->filename = calloc(1, sizeof(char) * (strlen(file) + 1));
    ogg_stream->file = fopen(resource_path(file), "rb");
    check(ogg_stream->file != NULL, "Couldn't open OGG file");

    check(OggStream_open_decoder(ogg_stream), "Couldn't open decoder");
    if (ogg_stream->vorbis_info->channels == 1) {
        ogg_stream->format = AL_FORMAT_MONO16;
    } else {
        ogg_stream->format = AL_FORMAT_STEREO16;
    }

    ogg_stream->source = source;
    alGenBuffers(2, ogg_stream->buffers);
    Audio_check();

    return ogg_stream;
error:
    if (ogg_stream && ogg_stream->file) fclose(ogg_stream->file);
    if (ogg_stream) free(ogg_stream);
    return NULL;
}

OggStream *OggStream_clone(OggStream *ogg_stream) {
    check(ogg_stream != NULL, "No ogg stream to clone");
    return OggStream_create(ogg_stream->filename, ogg_stream->source);
error:
    return NULL;
}


void OggStream_destroy(OggStream *ogg_stream) {
    check(ogg_stream != NULL, "No OGG stream to destroy");
    free(ogg_stream->filename);
    OggStream_empty(ogg_stream);

    alDeleteBuffers(2, ogg_stream->buffers);
    Audio_check();

    OggStream_close_decoder(ogg_stream);
    free(ogg_stream);
error:
    return;
}

void OggStream_debug(OggStream *ogg_stream) {
    printf("Version         %d\n", ogg_stream->vorbis_info->version);
    printf("Channels        %d\n", ogg_stream->vorbis_info->channels);
    printf("rate (hz)       %ld\n", ogg_stream->vorbis_info->rate);
    printf("bitrate upper   %ld\n", ogg_stream->vorbis_info->bitrate_upper);
    printf("bitrate nominal %ld\n", ogg_stream->vorbis_info->bitrate_nominal);
    printf("bitrate lower   %ld\n", ogg_stream->vorbis_info->bitrate_lower);
    printf("bitrate window  %ld\n\n", ogg_stream->vorbis_info->bitrate_window);
    printf("vendor  %s\n", ogg_stream->vorbis_comment->vendor);

    int i = 0;
    for (i = 0; i < ogg_stream->vorbis_comment->comments; i++) {
        printf("    %s\n", ogg_stream->vorbis_comment->user_comments[i]);
    }
}

int OggStream_play(OggStream *ogg_stream) {
    if (!ogg_stream) return 0;
    if (OggStream_playing(ogg_stream)) return 1;
    ogg_stream->buf_count = 1;

    check(OggStream_stream(ogg_stream, ogg_stream->buffers[0]) == 1,
        "Failed to stream OGG buffer 0");
    if (ogg_stream->stream.end > BUFFER_SIZE) {
        check(OggStream_stream(ogg_stream, ogg_stream->buffers[1]) == 1,
            "Failed to stream OGG buffer 1");
        ogg_stream->buf_count++;
    }

    alSourceQueueBuffers(ogg_stream->source, ogg_stream->buf_count,
            ogg_stream->buffers);
    alSourcePlay(ogg_stream->source);

    return 1;
error:
    return 0;
}

int OggStream_stop(OggStream *ogg_stream) {
    alSourceStop(ogg_stream->source);
    return Audio_check();
}

int OggStream_playing(OggStream *ogg_stream) {
    if (!ogg_stream) return 0;
    ALenum state;
    alGetSourcei(ogg_stream->source, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

int OggStream_update(OggStream *ogg_stream) {
    if (ogg_stream == NULL) return 0;
    int processed;
    int active = 1;

    ALenum state;
    alGetSourcei(ogg_stream->source, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED) return 1;

    alGetSourcei(ogg_stream->source, AL_BUFFERS_PROCESSED, &processed);

    while (processed--) {
        ALuint buffer;

        alSourceUnqueueBuffers(ogg_stream->source, 1, &buffer);
        Audio_check();

        active = OggStream_stream(ogg_stream, buffer);
        if (!active) continue;

        alSourceQueueBuffers(ogg_stream->source, 1, &buffer);
        Audio_check();
    }

    if (state != AL_STOPPED) return active;
    if (ogg_stream->eof) {
      ogg_stream->ended = 1;
    } else {
      alSourcePlay(ogg_stream->source);
    }

    return active;
}

int OggStream_stream(OggStream *ogg_stream, ALuint buffer) {
    char data[BUFFER_SIZE];
    int size = 0;
    int section;
    int result;

    while (size < BUFFER_SIZE) {
        result = ov_read(&ogg_stream->stream, data + size, BUFFER_SIZE - size,
                0, 2, 1, &section);

        if (result > 0) {
            size += result;
        } else {
            if (result < 0) {
                debug("%s", OggStream_error_string(result));
                return 0;
            } else {
                ogg_stream->eof = 1;
                break;
            }
        }
    }

    if (size == 0) return 0;

    alBufferData(buffer, ogg_stream->format, data, size,
            ogg_stream->vorbis_info->rate);

    return Audio_check();
}

void OggStream_rewind(OggStream *ogg_stream) {
    check(ogg_stream != NULL, "No ogg_stream to rewind");
    ov_raw_seek(&ogg_stream->stream, 0);

    ogg_stream->eof = 0;
    ogg_stream->ended = 0;
    return;
error:
    return;
}

void OggStream_empty(OggStream *ogg_stream) {
    ALenum state = 0;
    alGetSourcei(ogg_stream->source, AL_SOURCE_STATE, &state);
    Audio_check();
    check(state == AL_STOPPED, "OGG stream must be stopped to empty");

    int queued;
    alGetSourcei(ogg_stream->source, AL_BUFFERS_QUEUED, &queued);

    while (queued--) {
        ALuint buffer[1];

        alSourceUnqueueBuffers(ogg_stream->source, 1, buffer);
        Audio_check();
    }
    return;
error:
    return;
}

const char *OggStream_error_string(int code) {
    switch(code) {
        case OV_EREAD:
            return "Read from media.";
        case OV_ENOTVORBIS:
            return "Not Vorbis data.";
        case OV_EVERSION:
            return "Vorbis version mismatch.";
        case OV_EBADHEADER:
            return "Invalid Vorbis header.";
        case OV_EFAULT:
            return "Internal logic fault (bug or heap/stack corruption)";
        default:
            return "Unknown OGG error.";
    }
}
