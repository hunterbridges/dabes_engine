#include "audio.h"
#include "music.h"
#include "sfx.h"

ALfloat listenerPos[]={0.0,0.0,4.0};
ALfloat listenerVel[]={0.0,0.0,0.0};
ALfloat listenerOri[]={0.0,0.0,1.0,0.0,1.0,0.0};

Audio *Audio_create() {
    Audio *audio = calloc(1, sizeof(Audio));
    check(audio != NULL, "Couldn't create audio");
#ifdef DABES_IOS
#else
   int audio_rate = 44100;
    Uint16 audio_format = AUDIO_S16;
    int audio_channels = 2;
    int audio_buffers = 4096;
    if (Mix_OpenAudio(audio_rate, audio_format, audio_channels, audio_buffers)) {
        printf("Unable to open audio!\n");
        return 0;
    }
#endif
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

void Audio_stream(Audio *audio) {
    ListNode *node = audio->musics->first;
    while (node != NULL) {
        Music *music = node->value;

        if (music->ended) {
            ListNode *old = node;
            node = node->next;
            List_remove(audio->musics, old);
            Music_destroy(music);
            continue;
        }

        Music_update(music);
        node = node->next;
    }

    node = audio->active_sfx->first;
    while (node != NULL) {
        Sfx *sfx = node->value;

        if (sfx->ended) {
            ListNode *old = node;
            node = node->next;
            List_remove(audio->active_sfx, old);
            Sfx_destroy(sfx);
            continue;
        }

        Sfx_update(sfx);
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

#ifdef DABES_IOS
    alcSuspendContext(audio->context);
    alcMakeContextCurrent(NULL);
#endif
    alcDestroyContext(audio->context);
    alcCloseDevice(audio->device);
    free(audio);
error:
    return;
}

Music *Audio_gen_music(Audio *audio, int num_files, const char *files[]) {
    check(audio != NULL, "No audio to generate music");
    Music *music = Music_load(num_files, (char **)files);
    List_push(audio->musics, music);
    return music;
error:
    return NULL;
}

void Audio_destroy_music(Audio *audio, struct Music *music) {
  LIST_FOREACH(audio->musics, first, next, current) {
    if (current->value == music) {
      List_remove(audio->musics, current);
      Music_destroy(music);
      break;
    }
  }
}

struct Sfx *Audio_gen_sfx(Audio *audio, const char *filename) {
    check(audio != NULL, "No audio to generate sfx");
    Sfx *sfx = Sfx_load((char *)filename);
    List_push(audio->active_sfx, sfx);
    return sfx;
error:
    return NULL;
}

void Audio_destroy_sfx(Audio *audio, struct Sfx *sfx) {
  LIST_FOREACH(audio->active_sfx, first, next, current) {
    if (current->value == sfx) {
      List_remove(audio->active_sfx, current);
      Sfx_destroy(sfx);
      break;
    }
  }
}
