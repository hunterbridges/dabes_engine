#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import "input.h"

@interface EngineViewController : GLKViewController <UIGestureRecognizerDelegate,
                                              GLKViewDelegate>

- (id)initWithTouchInput:(Input *)input;
- (void)willEnterForeground;
@end
