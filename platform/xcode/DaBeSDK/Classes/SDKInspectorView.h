//
//  SDKInspectoveView.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/19/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

extern NSString *kControlChangedNotification;

#import <Cocoa/Cocoa.h>
#import <DaBes-Mac/DaBes-Mac.h>
#import <DaBes-Mac/DABMacEngineViewController.h>

@interface SDKInspectorView : NSTabView

@property (nonatomic, strong) DABMacEngineViewController *engineVC;
@property (nonatomic, assign) Input *touchInput;

@end
