//
//  SDKiOSProjectEngineViewController.m
//  DaBeSDK
//
//  Created by Hunter Bridges on 7/2/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "SDKiOSProjectEngineViewController.h"

@implementation SDKiOSProjectEngineViewController

- (NSString *)bootScript {
  return @"scripts/boot.lua";
}

- (NSString *)projectPath {
  NSString *demoPath = [[NSBundle mainBundle] pathForResource:@"boxfall"
                                                       ofType:@"dabes"
                                                  inDirectory:@"demo/boxfall/"];
  return demoPath;
}

@end
