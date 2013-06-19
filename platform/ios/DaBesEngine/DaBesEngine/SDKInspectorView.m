//
//  SDKInspectoveView.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/19/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "SDKInspectorView.h"

NSString *kControlChangedNotification = @"kControlChangedNotification";

@implementation SDKInspectorView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

- (void)postControlChange {
  [[NSNotificationCenter defaultCenter]
      postNotificationName:kControlChangedNotification
      object:self];
}

- (IBAction)zoomSliderChanged:(id)sender {
  NSEvent *event = [[NSApplication sharedApplication] currentEvent];
  NSSlider *slider = sender;
  BOOL startingDrag = event.type == NSLeftMouseDown;
  BOOL endingDrag = event.type == NSLeftMouseUp;
  BOOL dragging = event.type == NSLeftMouseDragged;
  
  [self postControlChange];
  
  if (startingDrag || dragging) {
    slider.intValue = slider.intValue;
  }
  
  if (endingDrag) {
    slider.intValue = 0;
  }
  
  self.touchInput->cam_zoom = slider.intValue;
}

@end
