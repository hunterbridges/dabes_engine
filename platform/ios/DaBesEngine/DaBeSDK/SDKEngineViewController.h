#import <Cocoa/Cocoa.h>
#import <GLKit/GLKit.h>
#import "engine.h"
#import "scene.h"
#import "input.h"

extern NSString *kFrameEndNotification;
extern NSString *kNewSceneNotification;
extern NSString *kEntitySelectedNotification;

@interface SDKEngineViewController : NSViewController

@property (nonatomic, assign) Engine *engine;
@property (nonatomic, assign) Scene *scene;
@property (nonatomic, assign) Input *touchInput;

@end
