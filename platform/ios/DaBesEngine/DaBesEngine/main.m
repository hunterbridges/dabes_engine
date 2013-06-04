//
//  main.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 3/21/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <UIKit/UIKit.h>

#import "AppDelegate.h"
#import "engine.h"
#import "scene.h"
#import "world.h"
#import <lcthw/dbg.h>

int main(int argc, char *argv[])
{
  @autoreleasepool {
    UIApplicationMain(argc, argv, NSStringFromClass([UIApplication class]),
                      NSStringFromClass([AppDelegate class]));
  }
  return 0;
}
