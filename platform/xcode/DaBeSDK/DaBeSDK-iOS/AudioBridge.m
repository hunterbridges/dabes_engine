#import <AVFoundation/AVFoundation.h>
#import <Foundation/Foundation.h>
#import "AudioBridge.h"

@interface AudioBridgeObject : NSObject <AVAudioPlayerDelegate>

- (void)loadFile:(char *)filename;
- (void)loadLoop:(char *)filename;
- (void)play;
- (void)pause;
- (void)setVolume:(double)volume;

@end

@implementation AudioBridgeObject {
    double volume_;
    AVAudioPlayer *avSound_;
    NSURL *nextUp_;
}

- (void)loadURL:(NSURL *)url {
    NSError *error = nil;
    avSound_ = [[AVAudioPlayer alloc] initWithContentsOfURL:url
                                                      error:&error];
    avSound_.numberOfLoops = -1;
    if (error) {
        NSLog(@"%@", [error description]);
        avSound_ = nil;
    }
}

- (void)loadFile:(char *)filename {
    CFStringRef cfstr =
        CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
    NSString *str = (__bridge_transfer NSString *)cfstr;
    NSString *absPath = [[[NSBundle mainBundle] bundlePath]
                         stringByAppendingPathComponent:str];
    NSURL *url = [NSURL fileURLWithPath:absPath];
    volume_ = 1;
    [self loadURL:url];
}

- (void)loadLoop:(char *)filename {
    CFStringRef cfstr =
        CFStringCreateWithCString(NULL, filename, kCFStringEncodingUTF8);
    NSString *str = (__bridge_transfer NSString *)cfstr;
    NSString *absPath = [[[NSBundle mainBundle] bundlePath]
                         stringByAppendingPathComponent:str];
    NSURL *url = [NSURL fileURLWithPath:absPath];
    nextUp_ = url;
  
    avSound_.numberOfLoops = 0;
    avSound_.delegate = self;
}

- (void)play {
    if (![avSound_ isPlaying]) [avSound_ play];
}

- (void)pause {
    if ([avSound_ isPlaying]) [avSound_ pause];
}

- (void)setVolume:(double)volume {
    volume_ = volume;
    avSound_.volume = volume;
}

- (void)audioPlayerDidFinishPlaying:(AVAudioPlayer *)player
                       successfully:(BOOL)flag {
  if (flag && player == avSound_ && nextUp_) {
    [self loadURL:nextUp_];
    nextUp_ = nil;
    avSound_.volume = volume_;
    [self play];
  }
}

@end

void AudioBridge_load(struct AudioBridge *bridge, char *filename) {
  AudioBridgeObject *bridgeObject = [[AudioBridgeObject alloc] init];
  [bridgeObject loadFile:filename];
  bridge->bridge_object = CFBridgingRetain(bridgeObject);
}

void AudioBridge_loop(struct AudioBridge *bridge, char *filename) {
  AudioBridgeObject *bridgeObject =
      (__bridge AudioBridgeObject *)bridge->bridge_object;
  [bridgeObject loadLoop:filename];
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

