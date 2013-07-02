#ifndef __audio_bridge_h

struct AudioBridge {
    CFTypeRef bridge_object;
} AudioBridge;

void AudioBridge_load(struct AudioBridge *bridge, char *filename);
void AudioBridge_loop(struct AudioBridge *bridge, char *filename);
void AudioBridge_play(struct AudioBridge *bridge);
void AudioBridge_pause(struct AudioBridge *bridge);
void AudioBridge_set_volume(struct AudioBridge *bridge, double volume);
void AudioBridge_destroy(struct AudioBridge *bridge);

#endif
