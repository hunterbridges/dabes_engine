//
//  LuaSyntaxStyler.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/1/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "LuaSyntaxStyler.h"

const NSInteger kTabWidth = 4;

@interface LuaSyntaxStyler ()

@end

@implementation LuaSyntaxStyler

- (void)setText:(NSString *)text {
  _text = [text copy];
}

- (void)replaceThenRestyleCharactersInRange:(NSRange)range
                                 withString:(NSString *)string
                                styledRange:(NSRange *)styledRange
                               styledString:(NSString **)styledString {
  self.lastIndentJump = 0;
  
#pragma mark - Handle Backspace
  
  if (range.length == 1 && [string isEqualToString:@""]) {
    NSInteger lineStart = [self lineStartLocationForLocation:range.location + 1];
    NSRange line = NSMakeRange(lineStart, range.location - lineStart + 1);
    NSString *lineString = [self.text substringWithRange:line];
    if ([lineString stringByReplacingOccurrencesOfString:@" "
                                              withString:@""].length == 0) {
      NSScanner *indentScanner = [NSScanner scannerWithString:lineString];
      indentScanner.charactersToBeSkipped = nil;
      NSCharacterSet *space =
          [NSCharacterSet characterSetWithCharactersInString:@" "];
      NSString *indent = nil;
      [indentScanner scanCharactersFromSet:space intoString:&indent];
      if (indent.length > 0) {
        int delwidth = indent.length % kTabWidth;
        if (delwidth == 0) delwidth = kTabWidth;
        
        delwidth -= 1;
        self.lastIndentJump = -delwidth;
        range.location -= delwidth;
        range.length += delwidth;
      }
    }
    
#pragma mark - Handle Space
  
  } else if ([string isEqualToString:@" "]) {
    NSInteger lineStart = [self lineStartLocationForLocation:range.location];
    NSRange line = NSMakeRange(lineStart, range.location - lineStart);
    NSString *lineString = [self.text substringWithRange:line];
    if ([lineString stringByReplacingOccurrencesOfString:@" "
                                              withString:@""].length == 0) {
      NSScanner *indentScanner = [NSScanner scannerWithString:lineString];
      indentScanner.charactersToBeSkipped = nil;
      NSCharacterSet *space =
          [NSCharacterSet characterSetWithCharactersInString:@" "];
      NSString *indent = nil;
      [indentScanner scanCharactersFromSet:space intoString:&indent];
      int addwidth = 4 - indent.length % kTabWidth;
      self.lastIndentJump = addwidth - 1;
      NSMutableString *tabbed = [[NSMutableString alloc] init];
      for (int i = 0; i < addwidth; i++) {
        [tabbed appendString:@" "];
      }
      string = tabbed;
    }
    
#pragma mark - Handle Newline
  
  } else if (string.length > 0 &&
      [[string substringToIndex:1] isEqualToString:@"\n"]) {
    NSInteger lineStart = [self lineStartLocationForLocation:range.location];
    NSRange line = NSMakeRange(lineStart, range.location - lineStart);
    NSString *lineString = [self.text substringWithRange:line];
    
    NSScanner *indentScanner = [NSScanner scannerWithString:lineString];
    indentScanner.charactersToBeSkipped = nil;
    NSCharacterSet *space =
        [NSCharacterSet characterSetWithCharactersInString:@" "];
    NSString *indent = nil;
    [indentScanner scanCharactersFromSet:space intoString:&indent];
    self.lastIndentJump = indent.length;
    
    NSString *indentedReplace = nil;
    if (indent) {
       indentedReplace =
          [NSString stringWithFormat:@"\n%@%@", indent,
              [string substringFromIndex:1]];
    } else {
       indentedReplace =
          [NSString stringWithFormat:@"\n%@", [string substringFromIndex:1]];
    }
    self.text = [self.text stringByReplacingCharactersInRange:range
                                                   withString:indentedReplace];
    *styledRange = range;
    *styledString = indentedReplace;
    return;
  }
  
  self.text = [self.text stringByReplacingCharactersInRange:range
                                                 withString:string];
  
  *styledRange = range;
  *styledString = string;
}

- (NSInteger)lineNumberOfLocation:(NSInteger)location {
  NSString *before = [self.text substringToIndex:location];
  int strCount = [before length] -
      [[before stringByReplacingOccurrencesOfString:@"\n" withString:@""] length];
  return strCount / [@"\n" length];
}

- (NSInteger)lineStartLocationForLocation:(NSInteger)location {
  NSString *before = [self.text substringToIndex:location];
  NSRange found = [before rangeOfString:@"\n" options:NSBackwardsSearch];
  if (found.location == NSNotFound) return 0;
  return found.location + 1;
}

@end
