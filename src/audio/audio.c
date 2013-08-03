#include "../scripting/scripting.h"
#include "../core/engine.h"
#include "audio.h"
#include "music.h"
#include "sfx.h"

ALfloat listenerPos[]={0.0,0.0,4.0};
ALfloat listenerVel[]={0.0,0.0,0.0};
ALfloat listenerOri[]={0.0,0.0,1.0,0.0,1.0,0.0};

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
    return 0;
}

void Audio_sweep(Audio *audio, Engine *engine) {
    ListNode *node = audio->musics->first;
    while (node != NULL) {
        Music *music = node->value;
        if (music->ended) {
            printf("Audio_sweep(): Sweeping music %p\n", music);
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

    node = audio->active_sfx->first;
    while (node != NULL) {
        Sfx *sfx = node->value;

        if (sfx->ended) {
            printf("Audio_sweep(): Sweeping sfx %p\n", sfx);
            ListNode *old = node;
            node = node->next;
            Scripting_call_hook(engine->scripting, sfx, "ended");
            List_remove(audio->active_sfx, old);
            Sfx_destroy(sfx);
            continue;
        }
        node = node->next;
    }
}

void Audio_t_stream(Audio *audio) {
    ListNode *node = audio->musics->first;
    while (node != NULL) {
        Music *music = node->value;
        Music_t_update(music);
        node = node->next;
    }

    node = audio->active_sfx->first;
    while (node != NULL) {
        Sfx *sfx = node->value;
        Sfx_t_update(sfx);
        node = node->next;
    }
}

void Audio_destroy(Audio *audio) {
    check(audio != NULL, "No audio to destroy");
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
error:
    return;
}

Music *Audio_gen_music(Audio *audio, int num_files, char *files[]) {
    check(audio != NULL, "No audio to generate music");
    Music *music = Music_load(num_files, files);
    check(music != NULL, "Couldn't load music");
    List_push(audio->musics, music);
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
    List_push(audio->active_sfx, sfx);
    return sfx;
error:
    return NULL;
}

void Audio_destroy_sfx(Audio *audio, struct Sfx *sfx) {
    int result = List_remove_value(audio->active_sfx, sfx);
    if (result) Sfx_destroy(sfx);
}
