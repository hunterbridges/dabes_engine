#import <Cocoa/Cocoa.h>
#import "engine.h"

@interface SDKEngineView : NSOpenGLView

@property (nonatomic, assign) Engine *engine;

- (void)update;

@end
