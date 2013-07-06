#import <Cocoa/Cocoa.h>
#import <DaBes-Mac/DaBes-Mac.h>

@interface DABMacEngineView : NSOpenGLView

@property (nonatomic, assign) Engine *engine;
@property (nonatomic, assign) Scene *scene;

- (id)initWithFrame:(NSRect)frameRect
        pixelFormat:(NSOpenGLPixelFormat *)format
         touchInput:(Input *)touchInput;
- (void)update;

@end
