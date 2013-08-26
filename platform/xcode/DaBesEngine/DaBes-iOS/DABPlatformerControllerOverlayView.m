//
//  DABPlatformerControllerOverlayView.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 8/25/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "DABPlatformerControllerOverlayView.h"

static NSTimeInterval kDebounceDelay = 0.1;

@interface DABPlatformerControllerOverlayView ()

@property (nonatomic, strong) UIButton *leftButton;
@property (nonatomic, strong) UIButton *rightButton;
@property (nonatomic, strong) NSTimer *debouncer;
@property (nonatomic, assign) BOOL debounce;

@end

@implementation DABPlatformerControllerOverlayView

- (id)initWithFrame:(CGRect)frame
{
  self = [super initWithFrame:frame];
  if (self) {
    self.leftButton = [UIButton buttonWithType:UIButtonTypeCustom];
    self.rightButton = [UIButton buttonWithType:UIButtonTypeCustom];
    
    [self.leftButton addTarget:self
                        action:@selector(buttonPressed:)
              forControlEvents:UIControlEventTouchDown];
    [self.rightButton addTarget:self
                         action:@selector(buttonPressed:)
               forControlEvents:UIControlEventTouchDown];
    [self.leftButton addTarget:self
                        action:@selector(buttonReleased:)
              forControlEvents:UIControlEventTouchUpInside];
    [self.rightButton addTarget:self
                         action:@selector(buttonReleased:)
               forControlEvents:UIControlEventTouchUpInside];
    [self.leftButton addTarget:self
                        action:@selector(buttonReleased:)
              forControlEvents:UIControlEventTouchUpOutside];
    [self.rightButton addTarget:self
                         action:@selector(buttonReleased:)
               forControlEvents:UIControlEventTouchUpOutside];
    
    [self addSubview:self.leftButton];
    [self addSubview:self.rightButton];
  }
  return self;
}

- (void)buttonPressed:(id)sender
{
  UIButton *button = sender;
  if (self.debugMode) {
    button.backgroundColor = [UIColor colorWithWhite:1 alpha:0.05];
  }
  
  BOOL l = self.leftButton.highlighted;
  BOOL r = self.rightButton.highlighted;
  Controller *p1 = self.touchInput->controllers[0];
  
  if (l && r) {
    p1->jump = 1;
    if (self.debounce) {
      p1->dpad &= ~(CONTROLLER_DPAD_LEFT);
      p1->dpad &= ~(CONTROLLER_DPAD_RIGHT);
    }
  }
  
  if (l && !r) {
    p1->dpad |= CONTROLLER_DPAD_LEFT;
  }
  
  if (r && !l) {
    p1->dpad |= CONTROLLER_DPAD_RIGHT;
  }
  
  if (!self.debouncer) {
    self.debouncer = [NSTimer scheduledTimerWithTimeInterval:kDebounceDelay
                                                      target:self
                                                    selector:@selector(handleDebounce:)
                                                    userInfo:nil
                                                     repeats:NO];
    self.debounce = YES;
  }
}

- (void)handleDebounce:(NSTimer *)timer
{
  self.debouncer = nil;
  self.debounce = NO;
}

- (void)buttonReleased:(id)sender
{
  UIButton *button = sender;
  button.backgroundColor = [UIColor clearColor];
  
  BOOL l = self.leftButton.highlighted;
  if (button == self.leftButton) l = NO;
  
  BOOL r = self.rightButton.highlighted;
  if (button == self.rightButton) r = NO;
  
  Controller *p1 = self.touchInput->controllers[0];
  
  if (!l || !r) {
    p1->jump = 0;
  }
  
  if (!l) {
    p1->dpad &= ~(CONTROLLER_DPAD_LEFT);
    if (r) {
      p1->dpad |= CONTROLLER_DPAD_RIGHT;
    }
  }
  
  if (!r) {
    p1->dpad &= ~(CONTROLLER_DPAD_RIGHT);
    if (l) {
      p1->dpad |= CONTROLLER_DPAD_LEFT;
    }
  }
}

- (void)layoutSubviews
{
  [super layoutSubviews];
  self.leftButton.frame = CGRectMake(0, 0, self.bounds.size.width / 2.0,
                                     self.bounds.size.height);
  self.rightButton.frame = CGRectMake(self.bounds.size.width / 2.0, 0,
                                      self.bounds.size.width / 2.0,
                                      self.bounds.size.height);
}

@end
