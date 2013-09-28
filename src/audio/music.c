#include "music.h"
#include <stdlib.h>
#include "../scenes/scene.h"
#include "../audio/audio.h"

void Music_t_end(Music *music);
void Music_t_initialize(Music *music);
void Music_t_uninitialize(Music *music);

#pragma mark - Main thread

Music *Music_load(int num_files, char *ogg_files[]) {
    Music *music = calloc(1, sizeof(Music) + num_files * sizeof(char *));
    check(music != NULL, "Couldn't create Music");
    music->volume = 1;
    music->loop = 1;
    music->ogg_streams = List_create();
    music->num_files = num_files;

    int rc = pthread_mutex_init(&music->lock, NULL);
    check (rc == 0, "Error making music->lock mutex");

    int i = 0;
    for (i = 0; i < num_files; i++) {
        char *filename = (char*)ogg_files[i];
        if (filename == NULL) continue;
        FILE *fcheck = fopen(filename, "r");
        check(fcheck != NULL, "Failed to find %s", filename);
        fclose(fcheck);

        music->ogg_files[i] = calloc(1, sizeof(char) * (strlen(filename) + 1));
        strcpy(music->ogg_files[i], filename);
    }

    return music;
error:
    if (music) {
      Music_destroy(music);
    }
    return NULL;
}

void Music_destroy(Music *music) {
    assert(music->initialized == 0);
    int i = 0;
    for (i = 0; i < music->num_files; i++) {
      free(music->ogg_files[i]);
    }
    Audio_check();
    pthread_mutex_destroy(&music->lock);
    free(music);
}

void Music_end(Music *music) {
    if (music->ended) return;
    pthread_mutex_lock(&music->lock);
    music->_needs_end = 1;
    pthread_mutex_unlock(&music->lock);
}

void Music_play(Music *music) {
    if (music == NULL) return;
    if (music->playing) return;
    pthread_mutex_lock(&music->lock);
    music->_needs_play = 1;
    pthread_mutex_unlock(&music->lock);
}

void Music_pause(Music *UNUSED(music)) {
    // TODO: Pause OGG
}

void Music_set_volume(Music *music, double volume) {
    pthread_mutex_lock(&music->lock);
    music->volume = volume;
    if (music->initialized) {
        // TODO: Lock? This might be thread safe
        alSourcef(music->source, AL_GAIN, volume * music->vol_adjust);
    }
    pthread_mutex_unlock(&music->lock);
}

void Music_set_vol_adjust(Music *music, double vol_adjust) {
    pthread_mutex_lock(&music->lock);
    music->vol_adjust = vol_adjust;
    if (music->initialized) {
        // TODO: Lock? This might be thread safe
        alSourcef(music->source, AL_GAIN, music->volume * vol_adjust);
    }
    pthread_mutex_unlock(&music->lock);
}

void Music_set_loop(Music *music, int loop) {
    pthread_mutex_lock(&music->lock);
    music->loop = loop;
    // TODO: Lock
    if (music->ogg_streams->last) {
        OggStream *last_stream = (OggStream *)music->ogg_streams->last->value;
        last_stream->should_loop = loop;
    }
    pthread_mutex_unlock(&music->lock);
}

#pragma mark - Audio Thread

void Music_t_end(Music *music) {
    log_info("Music_t_end(): Ending music...");
    alSourceStop(music->source);
    Music_t_uninitialize(music);
    music->ended = 1;
}

void Music_t_update(Music *music) {
    // TODO: Try Lock
    if (pthread_mutex_trylock(&music->lock)) return;

    if (music->ended) {
        pthread_mutex_unlock(&music->lock);
        return;
    }
    if (music->_needs_end) {
        Music_t_end(music);
        music->_needs_end = 0;
        pthread_mutex_unlock(&music->lock);
        return;
    }

    if (!music->initialized) {
        Music_t_initialize(music);
    }
    if (music->_needs_play) {
        music->_needs_play = 0;
        music->playing = 1;
        ListNode *node = music->ogg_streams->first;
        if (node == NULL) {
            pthread_mutex_unlock(&music->lock);
            return;
        }
        music->active_stream = node->value;
        OggStream_play(node->value);
    }

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
    if (music->active_stream && !music->active_stream->eof) {
        pthread_mutex_unlock(&music->lock);
        return;
    }

    if (active_node && active_node->next) {
        // Next? Should enqueue next
        ListNode *next_node = active_node->next;
        OggStream *next_stream = next_node->value;
        OggStream_play(next_stream);
        music->active_stream = next_stream;
    } else if (music->active_stream && music->active_stream->should_loop) {
        // Loop!
        OggStream_rewind(music->active_stream);
        OggStream_play(music->active_stream);
    }

    if (music->active_stream && music->active_stream->ended) {
        Music_t_end(music);
    }

    pthread_mutex_unlock(&music->lock);
}

void Music_t_initialize(Music *music) {
    assert(music->initialized == 0);

    alGenSources(1, &music->source);
    Audio_check();

    alSource3f(music->source, AL_POSITION, 0.0, 0.0, 0.0);
    alSource3f(music->source, AL_VELOCITY, 0.0, 0.0, 0.0);
    alSource3f(music->source, AL_DIRECTION, 0.0, 0.0, 0.0);
    alSourcef(music->source, AL_ROLLOFF_FACTOR, 0.0);
    alSourcei(music->source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcef(music->source, AL_GAIN,
              music->volume * music->vol_adjust);

    int i = 0;
    for (i = 0; i < music->num_files; i++) {
        char *filename = (char*)music->ogg_files[i];
        OggStream *ogg_stream = OggStream_create(filename,
                                                 music->source);
        check(ogg_stream != NULL, "Failed to create OGG Stream: %s", filename);
        List_push(music->ogg_streams, ogg_stream);
        if (i == music->num_files - 1 && music->loop) {
            ogg_stream->should_loop = 1;
        }
    }

    music->initialized = 1;
    return;
error:
    return;
}

void Music_t_uninitialize(Music *music) {
    assert(music->initialized == 1);
    Audio_check();
    alSourcei(music->source, AL_BUFFER, 0);
    Audio_check();

    LIST_FOREACH(music->ogg_streams, first, next, current) {
        OggStream *ogg_stream = current->value;
        OggStream_destroy(ogg_stream);
    }
    List_destroy(music->ogg_streams);
    music->ogg_streams = List_create();

    alDeleteSources(1, &music->source);
    music->source = 0;
    music->initialized = 0;
    log_info("Music_t_uninitialize(): Music uninitialized");
}
