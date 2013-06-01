#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import "input.h"
#import "engine.h"
#import "scene.h"

@interface EngineViewController : GLKViewController <UIGestureRecognizerDelegate,
                                              GLKViewDelegate>

- (id)initWithTouchInput:(Input *)input;
- (void)didEnterBackground;
- (void)willEnterForeground;
- (void)injectMapFromPath:(NSString *)newFilePath;
- (void)restartScene;
- (void)fullUpdate;
- (void)reboot;
  
@property (nonatomic, readonly) Engine *engine;
@property (nonatomic, readonly) Scene *scene;
@property (nonatomic, readonly) GLKView *glkView;
@end
