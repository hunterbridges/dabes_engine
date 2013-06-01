//
//  UIColor+LuaHighlighting.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "UIColor+LuaHighlighting.h"

@implementation UIColor (LuaHighlighting)

+ (UIColor *)codeBackgroundColor {
  return [UIColor colorWithHexRGB:0x252525];
}

+ (UIColor *)codeBackgroundErrorColor {
  return [UIColor colorWithHexRGB:0xc71e12];
}

+ (UIColor *)codeDefaultColor {
  return [UIColor colorWithHexRGB:0xD8D8D8];
}

+ (UIColor *)codeNumberColor {
  return [UIColor colorWithHexRGB:0xdebc9a];
}

+ (UIColor *)codeBooleanColor {
  return [UIColor colorWithHexRGB:0xfefbbf];
}

+ (UIColor *)codeStringColor {
  return [UIColor colorWithHexRGB:0xfefbbf];
}

+ (UIColor *)codeControlFlowColor {
  return [UIColor colorWithHexRGB:0x99bddd];
}

+ (UIColor *)codeKeywordColor {
  return [UIColor colorWithHexRGB:0xdebfdd];
}

+ (UIColor *)codeCommentColor {
  return [UIColor colorWithHexRGB:0x929292];
}

@end
