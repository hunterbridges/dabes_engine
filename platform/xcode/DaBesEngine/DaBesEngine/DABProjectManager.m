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

- (NSString *)createProjectInDirectory:(NSString *)path {
  // Derive project name
  NSFileManager *fileManager = [[NSFileManager alloc] init];
  
  NSString *enclosingDir = [path lastPathComponent];
  NSString *projectName = [enclosingDir stringByAppendingPathExtension:@"dabes"];
  NSString *projectPath = [path stringByAppendingPathComponent:projectName];
  
  if ([fileManager fileExistsAtPath:projectPath]) {
    return nil;
  }
  
  // Create project file (Blank for now)
  NSMutableData *data = [NSMutableData data];
  char nul = '\0';
  [data appendBytes:&nul length:1];
  [fileManager createFileAtPath:projectPath
                       contents:data
                     attributes:nil];
  
  // Create resource dirs
  NSArray *dirs = @[
    @"scripts",
    @"media",
    @"media/bgs",
    @"media/fonts",
    @"media/music",
    @"media/sfx",
    @"media/sprites",
    @"media/tilemaps",
    @"media/tilesets"
  ];
  for (NSString *dir in dirs) {
    NSString *dirpath = [path stringByAppendingPathComponent:dir];
    NSError *error = nil;
    [fileManager createDirectoryAtPath:dirpath
           withIntermediateDirectories:NO
                            attributes:nil
                                 error:&error];
    if (error) {
      NSLog(@"%@", error.description);
    }
  }
  
  // Create boot script
  NSString *bootScript = @"require 'dabes.scene_manager'\n"
                         @"\n"
                         @"function boot()\n"
                         @"    -- Usually here you want to create a scene and push it onto the manager.\n"
                         @"    -- ex.\n"
                         @"    --\n"
                         @"    -- local my_scene = MyScene:new()\n"
                         @"    -- scene_manager:push_scene(my_scene)\n"
                         @"end";
  NSData *scriptData = [bootScript dataUsingEncoding:NSUTF8StringEncoding];
  [fileManager createFileAtPath:[path stringByAppendingPathComponent:@"scripts/boot.lua"]
                       contents:scriptData
                     attributes:nil];
  return projectPath;
}

@end
