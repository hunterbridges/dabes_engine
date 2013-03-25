#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import "input.h"
#import "scene.h"

@interface EngineViewController : GLKViewController <UIGestureRecognizerDelegate,
                                              GLKViewDelegate>

- (id)initWithTouchInput:(Input *)input;
- (void)willEnterForeground;

@property (nonatomic, readonly) Scene *scene_;
@end
