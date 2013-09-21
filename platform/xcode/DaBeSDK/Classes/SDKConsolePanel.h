//
//  SDKConsolePanel.h
//  DaBeSDK
//
//  Created by Hunter Bridges on 9/21/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class SDKConsoleView;
@interface SDKConsolePanel : NSPanel

@property (nonatomic, weak) IBOutlet SDKConsoleView *consoleView;

@end
