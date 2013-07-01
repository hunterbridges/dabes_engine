//
//  DaBes_Mac.m
//  DaBes-Mac
//
//  Created by Hunter Bridges on 6/30/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "DaBes-Mac.h"

const char *resource_path(const char *filename) {
  NSString *nsFilename = [NSString stringWithCString:filename
                                            encoding:NSUTF8StringEncoding];
  
  NSFileManager *manager = [NSFileManager defaultManager];
  NSString *bundlePath = [[[NSBundle mainBundle] resourcePath]
                            stringByAppendingPathComponent:nsFilename];
  NSArray *docsPath =
      NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                          NSUserDomainMask, YES);
  NSString *docsFile = [[docsPath objectAtIndex:0]
                       stringByAppendingPathComponent:nsFilename];
  
  const char *cFullpath;
  if ([manager fileExistsAtPath:docsFile]) {
    cFullpath = [docsFile cStringUsingEncoding:NSUTF8StringEncoding];
  } else if ([manager fileExistsAtPath:bundlePath]) {
    cFullpath = [bundlePath cStringUsingEncoding:NSUTF8StringEncoding];
  } else {
    cFullpath = [bundlePath cStringUsingEncoding:NSUTF8StringEncoding];
  }
  
  return cFullpath;
}

FILE *load_resource(char *filename) {
    const char *cFullpath = resource_path(filename);
    FILE *file = fopen(cFullpath, "r");
    return file;
}

