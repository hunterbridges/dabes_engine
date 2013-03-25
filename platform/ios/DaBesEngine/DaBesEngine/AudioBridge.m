#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import "AudioBridge.h"

@interface AudioBridgeObject : NSObject

- (void)loadFile:(char *)filename;
- (void)play;
- (void)pause;
- (void)setVolume:(double)volume;

@end

@implementation AudioBridgeObject {
    AVAudioPlayer *avSound_;
}

- (void)loadFile:(char *)filename {
    CFStringRef cfstr =
        CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
    NSString *str = (__bridge_transfer NSString *)cfstr;
    NSString *absPath = [[[NSBundle mainBundle] bundlePath]
                         stringByAppendingPathComponent:str];
    NSURL *url = [NSURL fileURLWithPath:absPath];
    NSError *error = nil;
    avSound_ = [[AVAudioPlayer alloc] initWithContentsOfURL:url
                                                      error:&error];
    if (error) {
        NSLog(@"%@", [error description]);
        avSound_ = nil;
    }
}

- (void)play {
    if (![avSound_ isPlaying]) [avSound_ play];
}

- (void)pause {
    if ([avSound_ isPlaying]) [avSound_ pause];
}

- (void)setVolume:(double)volume {
    avSound_.volume = volume;
}

@end

void AudioBridge_load(struct AudioBridge *bridge, char *filename) {
  AudioBridgeObject *bridgeObject = [[AudioBridgeObject alloc] init];
  [bridgeObject loadFile:filename];
  bridge->bridge_object = CFBridgingRetain(bridgeObject);
}

void AudioBridge_play(struct AudioBridge *bridge) {
  AudioBridgeObject *bridgeObject =
      (__bridge AudioBridgeObject *)bridge->bridge_object;
  [bridgeObject play];
}

void AudioBridge_pause(struct AudioBridge *bridge) {
  AudioBridgeObject *bridgeObject =
      (__bridge AudioBridgeObject *)bridge->bridge_object;
  [bridgeObject pause];
}

void AudioBridge_set_volume(struct AudioBridge *bridge, double volume) {
  AudioBridgeObject *bridgeObject =
      (__bridge AudioBridgeObject *)bridge->bridge_object;
  [bridgeObject setVolume:volume];
}

void AudioBridge_destroy(struct AudioBridge *bridge) {
  CFBridgingRelease(bridge->bridge_object);
  free(bridge);
}

