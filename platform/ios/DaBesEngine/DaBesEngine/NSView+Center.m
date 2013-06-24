//
//  NSView+Center.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/20/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "NSView+Center.h"

@implementation NSView (Center)

- (void)setCenter:(CGPoint)center {
  CGRect newFrame = {center.x - self.frame.size.width / 2.0,
                     center.y - self.frame.size.height / 2.0,
                     self.frame.size};
  self.frame = NSRectFromCGRect(newFrame);
}

- (CGPoint)center {
  return CGPointMake(self.frame.origin.x + self.frame.size.width / 2.0,
                     self.frame.origin.y + self.frame.size.height / 2.0);
}
@end
