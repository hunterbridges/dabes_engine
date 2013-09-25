//
//  SDKConsolePanel.h
//  DaBeSDK
//
//  Created by Hunter Bridges on 9/21/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SDKConsolePanel : NSPanel

@property (nonatomic, assign) IBOutlet NSTextView *consoleView;


- (void)log:(NSString *)message withLevel:(NSString *)level;

@end
