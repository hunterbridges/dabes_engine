#import "UIColor+Hex.h"

@implementation UIColor (Hex)

+ (UIColor *)colorWithHexRGB:(uint32_t)rgb
{
    uint8_t r = (rgb & 0xFF0000) >> 16;
    uint8_t g = (rgb & 0x00FF00) >> 8;
    uint8_t b = (rgb & 0x0000FF);
    float rc = r / 255.0;
    float gc = g / 255.0;
    float bc = b / 255.0;
    return [UIColor colorWithRed:rc green:gc blue:bc alpha:1.0];
}

+ (UIColor *)colorWithHexRGBA:(uint32_t)rgba
{
    uint8_t r = (rgba & 0xFF000000) >> 24;
    uint8_t g = (rgba & 0x00FF0000) >> 16;
    uint8_t b = (rgba & 0x0000FF00) >> 8;
    uint8_t a = (rgba & 0x000000FF);
    float rc = r / 255.0;
    float gc = g / 255.0;
    float bc = b / 255.0;
    float ac = a / 255.0;
    return [UIColor colorWithRed:rc green:gc blue:bc alpha:ac];
}

@end
