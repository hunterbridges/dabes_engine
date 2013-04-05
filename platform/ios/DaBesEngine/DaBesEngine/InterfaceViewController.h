#import <UIKit/UIKit.h>
#import "input.h"
#import "tile_map.h"

typedef enum {
  GraphicalResourceKindSprite = 0,
  GraphicalResourceKindTileset = 1
} GraphicalResourceKind;

@interface InterfaceViewController : UIViewController <UIScrollViewDelegate,
  UITextFieldDelegate, UIImagePickerControllerDelegate, UIAlertViewDelegate,
  UIScrollViewDelegate, UINavigationControllerDelegate> {
    Input *touchInput_;
}

- (void)didEnterBackground;
- (void)willEnterForeground;
- (void)log:(NSString *)fmt arguments:(va_list)arguments;
- (void)injectMapFromPath:(NSString *)newFilePath;

@end
