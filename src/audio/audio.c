#include "../scripting/scripting.h"
#include "../core/engine.h"
#include "audio.h"
#include "music.h"
#include "sfx.h"

ALfloat listenerPos[]={0.0,0.0,4.0};
ALfloat listenerVel[]={0.0,0.0,0.0};
ALfloat listenerOri[]={0.0,0.0,1.0,0.0,1.0,0.0};

int Audio_t_stream(Audio *audio);
void Audio_t_create(Audio *audio);
void Audio_t_join(Audio *audio);

Audio *Audio_create() {
    Audio *audio = calloc(1, sizeof(Audio));
    check(audio != NULL, "Couldn't create audio");
    const ALCchar *defaultDevice =
        alcGetString(NULL, ALC_DEFAULT_DEVICE_SPECIFIER);
    check(defaultDevice != NULL, "Could not get default audio device");

    audio->device = alcOpenDevice(defaultDevice);
    check(audio->device != NULL, "Could not open audio device");

    audio->context = alcCreateContext(audio->device, NULL);
    check(audio->context != NULL, "Could not open audio context");

    check(alcMakeContextCurrent(audio->context) == AL_TRUE,
          "Could not make audio context current");

    Audio_check();
    alListenerfv(AL_POSITION,listenerPos);
    Audio_check();
    alListenerfv(AL_VELOCITY,listenerVel);
    Audio_check();
    alListenerfv(AL_ORIENTATION,listenerOri);
    Audio_check();

    audio->musics = List_create();
    audio->active_sfx = List_create();

    audio->master_volume = 1.0;
    audio->music_volume = 1.0;
    audio->sfx_volume = 1.0;

    Audio_t_create(audio);

    return audio;
error:
    if (audio) free(audio);
    return NULL;
}

int Audio_check() {
    ALenum error = alGetError();
    check(error == AL_NO_ERROR, "OpenAL error code: %d", error);
    return 1;
error:
    switch (error) {
      case AL_INVALID_VALUE:
        log_err("OpenAL AL_INVALID_VALUE");
        break;

      case AL_INVALID_OPERATION:
        log_err("OpenAL AL_INVALID_OPERATION");
        break;

      case AL_INVALID_NAME:
        log_err("OpenAL AL_INVALID_NAME");
        break;

      case AL_INVALID_ENUM:
        log_err("OpenAL AL_INVALID_ENUM");
        break;

      default:
        log_err("OpenAL error code %X", error);
        break;
    }

    return 0;
}

void Audio_sweep(Audio *audio, Engine *engine) {
    pthread_mutex_lock(&audio->music_lock);
    ListNode *node = audio->musics->first;
    while (node != NULL) {
        Music *music = node->value;
        if (music->ended) {
            ListNode *old = node;
            node = node->next;
            Scripting_call_hook(engine->scripting, music, "ended");
            Scripting_call_hook(engine->scripting, music, "_zero");
            List_remove(audio->musics, old);
            Music_destroy(music);
            continue;
        }
        node = node->next;
    }
    pthread_mutex_unlock(&audio->music_lock);

    pthread_mutex_lock(&audio->sfx_lock);
    node = audio->active_sfx->first;
    while (node != NULL) {
        Sfx *sfx = node->value;

        if (sfx->ended) {
            ListNode *old = node;
            node = node->next;
            Scripting_call_hook(engine->scripting, sfx, "ended");
            List_remove(audio->active_sfx, old);
            Sfx_destroy(sfx);
            continue;
        }
        node = node->next;
    }
    pthread_mutex_unlock(&audio->sfx_lock);
}

void Audio_destroy(Audio *audio) {
    check(audio != NULL, "No audio to destroy");

    Audio_t_join(audio);

    log_info("Audio_destroy(): Cleaning up Audio");

    {LIST_FOREACH(audio->musics, first, next, current) {
        Music *music = current->value;
        Music_destroy(music);
    }}
    List_destroy(audio->musics);

    {LIST_FOREACH(audio->active_sfx, first, next, current) {
        Sfx *sfx = current->value;
        Sfx_destroy(sfx);
    }}
    List_destroy(audio->active_sfx);

#if defined(DABES_IOS) || defined(DABES_MAC)
    alcSuspendContext(audio->context);
    alcMakeContextCurrent(NULL);
#endif
    alcDestroyContext(audio->context);
    alcCloseDevice(audio->device);
    free(audio);

    log_info("Audio_destroy(): Audio destroyed");
error:
    return;
}

Music *Audio_gen_music(Audio *audio, int num_files, char *files[]) {
    check(audio != NULL, "No audio to generate music");
    Music *music = Music_load(num_files, files);
    check(music != NULL, "Couldn't load music");

    Music_set_vol_adjust(music, audio->music_volume * audio->master_volume);

    pthread_mutex_lock(&audio->music_lock);
    List_push(audio->musics, music);
    pthread_mutex_unlock(&audio->music_lock);
    return music;
error:
    return NULL;
}

void Audio_destroy_music(Audio *audio, struct Music *music) {
    int result = List_remove_value(audio->musics, music);
    if (result) Music_destroy(music);
}

struct Sfx *Audio_gen_sfx(Audio *audio, char *filename) {
    check(audio != NULL, "No audio to generate Sfx");
    Sfx *sfx = Sfx_load(filename);
    check(sfx != NULL, "Couldn't load Sfx");

    Sfx_set_vol_adjust(sfx, audio->sfx_volume * audio->master_volume);

    pthread_mutex_lock(&audio->sfx_lock);
    List_push(audio->active_sfx, sfx);
    pthread_mutex_unlock(&audio->sfx_lock);

    return sfx;
error:
    return NULL;
}

void Audio_destroy_sfx(Audio *audio, struct Sfx *sfx) {
    int result = List_remove_value(audio->active_sfx, sfx);
    if (result) Sfx_destroy(sfx);
}

void Audio_set_master_volume(Audio *audio, double vol) {
    audio->master_volume = vol;

    // Cascade the updates down.
    Audio_set_music_volume(audio, audio->music_volume);
    Audio_set_sfx_volume(audio, audio->sfx_volume);
}

void Audio_set_music_volume(Audio *audio, double vol) {
    audio->music_volume = vol;

    LIST_FOREACH(audio->musics, first, next, current) {
        Music *music = current->value;
        Music_set_vol_adjust(music, audio->music_volume * audio->master_volume);
    }
}

void Audio_set_sfx_volume(Audio *audio, double vol) {
    audio->sfx_volume = vol;

    LIST_FOREACH(audio->active_sfx, first, next, current) {
        Sfx *sfx = current->value;
        Sfx_set_vol_adjust(sfx, audio->sfx_volume * audio->master_volume);
    }
}

#pragma mark - Audio thread

void *Audio_t_work(void *threadarg) {
    Audio *audio = threadarg;

    int rc = 0;
    while (rc == 0) {
        rc = Audio_t_stream(audio);
    }

    log_info("Audio_t_work(): exiting...");
    pthread_exit(NULL);
}

void Audio_t_create(Audio *audio) {
    // Create run lock mutex
    log_info("Audio_t_create(): creating locks...");
    if (pthread_mutex_init(&audio->run_lock, NULL)) {
        log_err("Audio_t_create(): Couldn't create run_lock");
    }
    if (pthread_mutex_init(&audio->music_lock, NULL)) {
        log_err("Audio_t_create(): Couldn't create music_lock");
    }
    if (pthread_mutex_init(&audio->sfx_lock, NULL)) {
        log_err("Audio_t_create(): Couldn't create sfx_lock");
    }
    log_info("Audio_t_create(): Locking run_lock...");
    pthread_mutex_lock(&audio->run_lock);

    // Create joinable worker thread
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
    log_info("Audio_t_create(): creating thread...");
    int rc = pthread_create(&audio->thread, &attr, Audio_t_work, (void *)audio);
    if (rc) {
        log_err("return code from pthread_create() is %d", rc);
        exit(-1);
    } else {
        log_info("Audio_t_create(): Audio thread created on thread %p",
                audio->thread);
    }
    pthread_attr_destroy(&attr);
}

void Audio_t_join(Audio *audio) {
    log_info("Audio_t_join(): unlocking run_lock...");
    pthread_mutex_unlock(&audio->run_lock);

    void *status;
    log_info("Audio_t_join(): joining thread %p...", audio->thread);
    int rc = pthread_join(audio->thread, &status);
    // The main thread now waits for the audio thread to clean up...

    if (rc) {
        log_err("return code from pthread_join() is %d", rc);
        exit(-1);
    }

    log_info("Audio_t_join(): destroying locks...");
    pthread_mutex_destroy(&audio->run_lock);
    pthread_mutex_destroy(&audio->music_lock);
    pthread_mutex_destroy(&audio->sfx_lock);
}

int Audio_t_stream(Audio *audio) {
    int cleanup = 0;
    if (pthread_mutex_trylock(&audio->run_lock) == 0) {
        // If stream was able to get the run lock, that means we need to clean
        // up and exit.
        log_info("Audio_t_stream(): thread locked run_lock. Cleaning up...");
        cleanup = 1;
        if (pthread_mutex_unlock(&audio->run_lock)) {
            log_err("Couldn't unlock run_lock");
        }
    }

    pthread_mutex_lock(&audio->music_lock);
    ListNode *node = audio->musics->first;
    while (node != NULL) {
        Music *music = node->value;
        if (cleanup) Music_end(music);
        Music_t_update(music);
        node = node->next;
    }
    pthread_mutex_unlock(&audio->music_lock);

    pthread_mutex_lock(&audio->sfx_lock);
    node = audio->active_sfx->first;
    while (node != NULL) {
        Sfx *sfx = node->value;
        if (cleanup) Sfx_end(sfx);
        Sfx_t_update(sfx);
        node = node->next;
    }
    pthread_mutex_unlock(&audio->sfx_lock);

    return cleanup;
}

