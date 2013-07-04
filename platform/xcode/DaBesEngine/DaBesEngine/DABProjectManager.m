//
//  DABProjectManager.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 7/4/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "DABProjectManager.h"

char *DABProjectManager_path_func(const char *filename) {
  DABProjectManager *manager = [DABProjectManager sharedInstance];
  NSString *path =
      [NSString stringWithFormat:@"%@/%s", manager.projectDir, filename];
  const char *cFullpath = [path cStringUsingEncoding:NSUTF8StringEncoding];
  
  char *cpy = calloc(strlen(cFullpath) + 1, sizeof(char));
  strcpy(cpy, cFullpath);
  return cpy;
}

@implementation DABProjectManager

+ (DABProjectManager *)sharedInstance {
  static DABProjectManager *shared;
  static dispatch_once_t onceToken;
  dispatch_once(&onceToken, ^{
    shared = [[DABProjectManager alloc] init];
  });
  return shared;
}

- (NSString *)projectDir {
  return [self.projectPath stringByDeletingLastPathComponent];
}

@end
