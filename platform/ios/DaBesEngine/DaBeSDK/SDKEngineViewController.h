#import <Cocoa/Cocoa.h>
#import <GLKit/GLKit.h>
#import "engine.h"
#import "scene.h"
#import "input.h"

@interface SDKEngineViewController : NSViewController

@property (nonatomic, assign) Engine *engine;
@property (nonatomic, readonly) Scene *scene;
@property (nonatomic, assign) Input *touchInput;

@end
