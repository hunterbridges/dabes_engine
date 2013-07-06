//
//  FileSystemItem.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/28/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface FileSystemItem : NSObject
{
    NSString *basePath;
    NSString *relativePath;
    FileSystemItem *parent;
    NSMutableArray *children;
}

@property (nonatomic, assign) BOOL isDirectory;
 
+ (FileSystemItem *)rootItem;
- (NSInteger)numberOfChildren;// Returns -1 for leaf nodes
- (FileSystemItem *)childAtIndex:(NSUInteger)n; // Invalid to call on leaf nodes
- (NSString *)fullPath;
- (NSString *)relativePath;
- (void)refresh;
- (void)renameTo:(NSString *)newName;

- (id)initWithPath:(NSString *)path parent:(FileSystemItem *)parentItem;

@property (nonatomic, readonly) NSArray *children;
@property (nonatomic, readonly) FileSystemItem *parent;

@end