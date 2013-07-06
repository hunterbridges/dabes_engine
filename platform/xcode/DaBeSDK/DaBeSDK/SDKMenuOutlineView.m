//
//  SDKDirTreeOutlineView.m
//  DaBeSDK
//
//  Created by Hunter Bridges on 7/5/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "SDKMenuOutlineView.h"

@implementation SDKMenuOutlineView

- (NSMenu *)menuForEvent:(NSEvent *)theEvent {
  NSPoint pt = [self convertPoint:[theEvent locationInWindow]
                         fromView:nil];
  id item = [self itemAtRow:[self rowAtPoint:pt]];
  [self selectRowIndexes:[NSIndexSet indexSetWithIndex:[self rowAtPoint:pt]]
    byExtendingSelection:NO];
  return [self defaultMenuForItem:item];
}

- (NSMenu*)defaultMenuForItem:(id)item {
  return [self.delegate outlineView:self menuForItem:item];
}

- (void)setDelegate:(id<SDKMenuOutlineViewDelegate>)delegate {
  [super setDelegate:delegate];
}

- (id<SDKMenuOutlineViewDelegate>)delegate {
  return (id<SDKMenuOutlineViewDelegate>)[super delegate];
}

@end
