//
//  SDKProjectEngineViewController.m
//  DaBeSDK
//
//  Created by Hunter Bridges on 7/2/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "SDKProjectEngineViewController.h"

@interface SDKProjectEngineViewController ()

@end

@implementation SDKProjectEngineViewController

- (NSString *)bootScript {
  return @"scripts/boot.lua";
}

- (NSString *)projectPath {
  return @"/Users/hbridges/Coding/dabes_engine/demo/boxfall/boxfall.dabes";
}

@end
