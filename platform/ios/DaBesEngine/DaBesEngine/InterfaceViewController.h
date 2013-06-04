#import <UIKit/UIKit.h>
#import "input.h"
#import "tile_map.h"

@interface InterfaceViewController : UIViewController

- (void)didEnterBackground;
- (void)willEnterForeground;
- (void)log:(NSString *)fmt arguments:(va_list)arguments;
- (void)injectMapFromPath:(NSString *)newFilePath;

@end
