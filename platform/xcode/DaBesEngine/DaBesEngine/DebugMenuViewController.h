//
//  DebugMenuViewController.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/4/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "input.h"

typedef enum {
  GraphicalResourceKindSprite = 0,
  GraphicalResourceKindTileset = 1
} GraphicalResourceKind;

@class EngineViewController;
@interface DebugMenuViewController : UIViewController

@property (nonatomic, strong, readonly) UIScrollView *scrollView;

- (id)initWithEngineVC:(EngineViewController *)engineVC
        withTouchInput:(Input *)touchInput;
- (CGFloat)height;
- (int)pages;
- (void)log:(NSString *)fmt arguments:(va_list)arguments;

@end
