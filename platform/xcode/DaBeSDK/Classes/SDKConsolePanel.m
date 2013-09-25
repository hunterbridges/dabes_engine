//
//  SDKConsolePanel.m
//  DaBeSDK
//
//  Created by Hunter Bridges on 9/21/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "SDKConsolePanel.h"

@interface SDKConsolePanel ()
@property (nonatomic, strong) NSMutableArray *messages;
@end

@implementation SDKConsolePanel

- (void)log:(NSString *)message withLevel:(NSString *)level {
  [self.messages addObject:[message copy]];
  
  NSColor *color;
  NSFont *font = [NSFont fontWithName:@"Menlo" size:14.0];

  if ([level isEqualToString:@"WARN"]) {
    color = [NSColor orangeColor];
  } else if ([level isEqualToString:@"ERROR"]) {
    color = [NSColor redColor];
  } else {
    color = [NSColor blackColor];
  }
  
  NSDictionary *attrs = @{
                          NSForegroundColorAttributeName: color,
                          NSFontAttributeName: font
  };
  
  NSAttributedString *coloredMessage = [[NSAttributedString alloc] initWithString:[message copy] attributes: attrs];
  
  [self.consoleView insertText:coloredMessage];
  [self.consoleView insertText:@"\n"];
}

@end
