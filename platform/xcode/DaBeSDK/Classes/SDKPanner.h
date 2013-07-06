//
//  SDKPanner.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/20/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface SDKPanner : NSControl

@property (nonatomic, assign, readonly) CGPoint cgPointValue;

- (id)initWithCoder:(NSCoder *)aDecoder;
- (id)initWithFrame:(NSRect)frameRect;

@end
