//
//  SDKInspectoveView.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/19/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

extern NSString *kControlChangedNotification;

#import <Cocoa/Cocoa.h>
#import "engine.h"
#import "input.h"

@interface SDKInspectorView : NSTabView

@property (nonatomic, strong) IBOutlet NSSlider *cameraSlider;
@property (nonatomic, assign) Engine *engine;
@property (nonatomic, assign) Input *touchInput;

@end
