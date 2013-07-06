//
//  UIColor+LuaHighlighting.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "UIColor+Hex.h"

@interface UIColor (LuaHighlighting)

+ (UIColor *)codeBackgroundColor;
+ (UIColor *)codeBackgroundErrorColor;
+ (UIColor *)codeDefaultColor;
+ (UIColor *)codeNumberColor;
+ (UIColor *)codeBooleanColor;
+ (UIColor *)codeStringColor;
+ (UIColor *)codeControlFlowColor;
+ (UIColor *)codeKeywordColor;
+ (UIColor *)codeCommentColor;

@end
