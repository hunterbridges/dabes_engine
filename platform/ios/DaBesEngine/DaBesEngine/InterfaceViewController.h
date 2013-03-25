#import <UIKit/UIKit.h>
#import "input.h"

@interface InterfaceViewController : UIViewController <UIScrollViewDelegate> {
    Input *touchInput_;
}

- (void)willEnterForeground;

@end
