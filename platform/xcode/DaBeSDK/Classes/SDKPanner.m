//
//  SDKPanner.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/20/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "SDKPanner.h"
#import "NSView+Center.h"

@interface SDKPanner ()

@property (nonatomic, strong) NSBox *box;
@property (nonatomic, strong) NSButton *button;
@property (nonatomic, assign) CGPoint cgPointValue;
@property (nonatomic, assign) SEL action;
@property (nonatomic, assign) id target;
@property (nonatomic, assign) BOOL enabled;

@end

@implementation SDKPanner
@synthesize enabled = _enabled;

- (void)commonInit {
  NSBox *box = [[NSBox alloc] initWithFrame:self.bounds];
  box.autoresizingMask = (NSViewWidthSizable | NSViewHeightSizable);
  box.translatesAutoresizingMaskIntoConstraints = YES;
  box.title = @"";
  box.titlePosition = NSNoTitle;
  [self addSubview:box];
  self.box = box;
  
  NSButton *button = [[NSButton alloc] initWithFrame:CGRectMake(0, 0, 24, 24)];
  button.bezelStyle = NSCircularBezelStyle;
  button.frame = CGRectMake((self.bounds.size.width - button.frame.size.width) / 2.0,
                            (self.bounds.size.width - button.frame.size.width) / 2.0,
                            button.frame.size.width,
                            button.frame.size.height);
  [button setEnabled:NO];
  [button setTitle:@""];
  [self addSubview:button];
  self.button = button;
  
  self.enabled = YES;
}

- (id)initWithCoder:(NSCoder *)aDecoder {
  self = [super initWithCoder:aDecoder];
  if (self) {
    [self commonInit];
  }
  return self;
}

- (id)initWithFrame:(NSRect)frameRect {
  self = [super initWithFrame:frameRect];
  if (self) {
    [self commonInit];
  }
  return self;
}

- (void)trackMouse {
  NSPoint mouse = [self.window convertScreenToBase:[NSEvent mouseLocation]];
  NSPoint converted = [self convertPoint:mouse fromView:nil];
  CGPoint cgConv = NSPointToCGPoint(converted);
  cgConv.x = MIN(MAX(0, cgConv.x), self.bounds.size.width);
  cgConv.y = MIN(MAX(0, cgConv.y), self.bounds.size.height);
  self.button.center = cgConv;
  
  CGPoint unit = {cgConv.x / self.bounds.size.width - 0.5,
                  cgConv.y / self.bounds.size.height - 0.5};
  self.cgPointValue = unit;
  [self sendAction:self.action to:self.target];
}

- (void)mouseDown:(NSEvent *)theEvent {
  if (!self.enabled) return;
  [self trackMouse];
  [super mouseDown:theEvent];
  [self.button setEnabled:YES];
  [self.button highlight:YES];
}

- (void)mouseDragged:(NSEvent *)theEvent {
  if (!self.enabled) return;
  [self trackMouse];
}

- (void)mouseUp:(NSEvent *)theEvent {
  [super mouseUp:theEvent];
  self.button.center = self.box.center;
  [self.button setEnabled:NO];
  [self.button highlight:NO];
  self.cgPointValue = CGPointZero;
  [self sendAction:self.action to:self.target];
}

- (void)setEnabled:(BOOL)enabled {
  _enabled = enabled;
  if (enabled) {
    self.alphaValue = 1.0;
  } else {
    self.alphaValue = 0.5;
  }
}

@end
