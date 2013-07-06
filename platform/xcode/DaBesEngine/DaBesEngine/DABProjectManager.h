//
//  DABProjectManager.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 7/4/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Foundation/Foundation.h>

char *DABProjectManager_path_func(const char *filename);

@interface DABProjectManager : NSObject

@property (nonatomic, copy) NSString *projectPath;
@property (nonatomic, copy, readonly) NSString *projectDir;

+ (DABProjectManager *)sharedInstance;
- (NSString *)createProjectInDirectory:(NSString *)path;
  
@end
