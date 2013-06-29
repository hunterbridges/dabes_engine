//
//  FileSystemItem.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/28/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "FileSystemItem.h"

@implementation FileSystemItem
 
static FileSystemItem *rootItem = nil;
static NSMutableArray *leafNode = nil;
 
+ (void)initialize {
    if (self == [FileSystemItem class]) {
        leafNode = [[NSMutableArray alloc] init];
    }
}
 
- (id)initWithPath:(NSString *)path parent:(FileSystemItem *)parentItem {
    self = [super init];
    if (self) {
       basePath = [path copy];
       relativePath = [[path lastPathComponent] copy];
       parent = parentItem;
     }
    return self;
}
 
+ (FileSystemItem *)rootItem {
    if (rootItem == nil) {
        rootItem = [[FileSystemItem alloc] initWithPath:@"/" parent:nil];
    }
    return rootItem;
}
 
// Creates, caches, and returns the array of children
// Loads children incrementally
- (NSArray *)children {
  if (children == nil) {
    NSFileManager *fileManager = [NSFileManager defaultManager];
    NSString *fullPath = [self fullPath];
    BOOL isDir, valid;

    valid = [fileManager fileExistsAtPath:fullPath isDirectory:&isDir];

    if (valid && isDir) {
        NSArray *array = [fileManager contentsOfDirectoryAtPath:fullPath error:NULL];

        NSUInteger numChildren, i;

        numChildren = [array count];
        children = [[NSMutableArray alloc] initWithCapacity:numChildren];

        for (i = 0; i < numChildren; i++) {
          FileSystemItem *newChild = [[FileSystemItem alloc]
                             initWithPath:[array objectAtIndex:i] parent:self];
          [children addObject:newChild];
        }
    }
    else {
        children = leafNode;
    }
  }
  return children;
}
 
 
- (NSString *)relativePath {
    return relativePath;
}
 
 
- (NSString *)fullPath {
    // If no parent, return our own relative path
    if (parent == nil) {
        return basePath;
    }
 
    // recurse up the hierarchy, prepending each parent’s path
    return [[parent fullPath] stringByAppendingPathComponent:relativePath];
}
 
 
- (FileSystemItem *)childAtIndex:(NSUInteger)n {
  return [[self children] objectAtIndex:n];
}
 
 
- (NSInteger)numberOfChildren {
  NSArray *tmp = [self children];
  return (tmp == leafNode) ? (-1) : [tmp count];
}
 
 
- (void)dealloc {
  if (children != leafNode) {
    children = nil;
  }
  relativePath = nil;
}
 
@end