#import <UIKit/UIKit.h>

@interface UIColor (Hex)

+ (UIColor *)colorWithHexRGB:(uint32_t)rgb;
+ (UIColor *)colorWithHexRGBA:(uint32_t)rgba;

@end
