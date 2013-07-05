//
//  SDKProjectEngineViewController.m
//  DaBeSDK
//
//  Created by Hunter Bridges on 7/2/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "SDKProjectEngineViewController.h"
#import "SDKMacAppDelegate.h"

@interface SDKProjectEngineViewController ()

@end

@implementation SDKProjectEngineViewController

- (NSString *)bootScript {
  return @"scripts/boot.lua";
}

- (NSString *)projectPath {
  return ((SDKMacAppDelegate *)[NSApp delegate]).curentProject;
}

@end
