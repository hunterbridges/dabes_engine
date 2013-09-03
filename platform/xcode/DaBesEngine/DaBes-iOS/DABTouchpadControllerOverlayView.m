//
//  DABTouchpadControllerOverlayView.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 9/2/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "DABTouchpadControllerOverlayView.h"

@implementation DABTouchpadControllerOverlayView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        self.multipleTouchEnabled = NO;
    }
    return self;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesBegan:touches withEvent:event];
    UITouch *touch = [touches anyObject];
    
    CGPoint p = [touch locationInView:self];
    VPoint screen_point = {p.x, p.y};
    Input *input = self.touchInput;
    input->controllers[0]->touch_state |= CONTROLLER_TOUCH_HOLD_CHANGED;
    input->controllers[0]->touch_state |= CONTROLLER_TOUCH_HOLD;
    input->controllers[0]->touch_pos = screen_point;
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesEnded:touches withEvent:event];
    UITouch *touch = [touches anyObject];
    
    CGPoint p = [touch locationInView:self];
    VPoint screen_point = {p.x, p.y};
    Input *input = self.touchInput;
    input->controllers[0]->touch_state |= CONTROLLER_TOUCH_HOLD_CHANGED;
    input->controllers[0]->touch_state &= ~(CONTROLLER_TOUCH_HOLD);
    input->controllers[0]->touch_pos = screen_point;
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    [self touchesEnded:touches withEvent:event];
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    [super touchesMoved:touches withEvent:event];
    UITouch *touch = [touches anyObject];
    
    CGPoint p = [touch locationInView:self];
    VPoint screen_point = {p.x, p.y};
    
    Input *input = self.touchInput;
    input->controllers[0]->touch_state |= CONTROLLER_TOUCH_MOVED;
    input->controllers[0]->touch_pos = screen_point;
}

@end
