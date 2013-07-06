//
//  DebugMenuViewController.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/4/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <DaBes-iOS/DaBes-iOS.h>

typedef enum {
  GraphicalResourceKindSprite = 0,
  GraphicalResourceKindTileset = 1
} GraphicalResourceKind;

@class SDKiOSProjectEngineViewController;
@interface DebugMenuViewController : UIViewController

@property (nonatomic, strong, readonly) UIScrollView *scrollView;

- (id)initWithEngineVC:(SDKiOSProjectEngineViewController *)engineVC
        withTouchInput:(Input *)touchInput;
- (CGFloat)height;
- (int)pages;
- (void)log:(NSString *)fmt arguments:(va_list)arguments;

@end
