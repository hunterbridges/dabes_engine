#include "music.h"
#include <stdlib.h>
#include "../scenes/scene.h"

Music *Music_load(int num_files, char *ogg_files[]) {
    Music *music = malloc(sizeof(Music) + num_files * sizeof(char *));
    check(music != NULL, "Couldn't create Music");
    music->volume = 1;
    music->loop = 1;
    music->playing = 0;
    music->ended = 0;
    music->ogg_streams = List_create();
    music->num_files = num_files;

    alGenSources(1, &music->source);
    Audio_check();

    alSource3f(music->source, AL_POSITION, 0.0, 0.0, 0.0);
    alSource3f(music->source, AL_VELOCITY, 0.0, 0.0, 0.0);
    alSource3f(music->source, AL_DIRECTION, 0.0, 0.0, 0.0);
    alSourcef(music->source, AL_ROLLOFF_FACTOR, 0.0);
    alSourcei(music->source, AL_SOURCE_RELATIVE, AL_TRUE);

    int i = 0;
    for (i = 0; i < num_files; i++) {
        char *filename = (char*)ogg_files[i];
        if (filename == NULL) continue;
        music->ogg_files[i] = calloc(1, sizeof(char) * (strlen(filename) + 1));
        strcpy(music->ogg_files[i], filename);
        OggStream *ogg_stream = OggStream_create(filename,
                                                 music->source);
        check(ogg_stream != NULL, "Failed to create OGG Stream: %s", filename);
        List_push(music->ogg_streams, ogg_stream);
        if (i == num_files - 1) ogg_stream->should_loop = 1;
    }

    return music;
error:
    if (music) {
      Music_destroy(music);
    }
    return NULL;
}

void Music_end(Music *music) {
    if (music->ended) return;
    alSourceStop(music->source);
    music->ended = 1;
}

void Music_destroy(Music *music) {
    Music_end(music);
    int i = 0;
    for (i = 0; i < music->num_files; i++) {
      free(music->ogg_files[i]);
    }
    Audio_check();
    alSourcei(music->source, AL_BUFFER, 0);
    Audio_check();
    LIST_FOREACH(music->ogg_streams, first, next, current) {
        OggStream *ogg_stream = current->value;
        OggStream_destroy(ogg_stream);
    }
    List_destroy(music->ogg_streams);

    alDeleteSources(1, &music->source);
    Audio_check();
    free(music);
}

void Music_play(Music *music) {
    if (music == NULL) return;
    if (music->playing) return;
    ListNode *node = music->ogg_streams->first;
    if (node == NULL) return;
    music->active_stream = node->value;
    music->playing = 1;
    OggStream_play(node->value);
}

void Music_update(Music *music) {
    ListNode *node = music->ogg_streams->first;
    ListNode *active_node = NULL;
    while (node != NULL) {
        OggStream *stream = node->value;
        if (stream == music->active_stream) active_node = node;
        if (stream->ended) {
          ListNode *old = node;
          node = node->next;
          OggStream_destroy(stream);
          List_remove(music->ogg_streams, old);
          continue;
        }
        node = node->next;
    }

    OggStream_update(music->active_stream);
    if (!music->active_stream->eof) return;

    if (active_node && active_node->next) {
        // Next? Should enqueue next
        ListNode *next_node = active_node->next;
        OggStream *next_stream = next_node->value;
        OggStream_play(next_stream);
        music->active_stream = next_stream;
    } else if (music->active_stream->should_loop) {
        // Loop!
        OggStream_rewind(music->active_stream);
        OggStream_play(music->active_stream);
    }

    if (music->active_stream && music->active_stream->ended) {
        music->ended = 1;
    }
}

void Music_pause(Music *UNUSED(music)) {
    // TODO: Pause OGG
}

void Music_set_volume(Music *music, double volume) {
    music->volume = volume;
    alSourcef(music->source, AL_GAIN, volume);
}

void Music_set_loop(Music *music, int loop) {
    music->loop = loop;
    if (music->ogg_streams->last) {
        OggStream *last_stream = (OggStream *)music->ogg_streams->last->value;
        last_stream->should_loop = loop;
    }
}

