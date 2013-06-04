//
//  LuaSyntaxHighlighter.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <CoreText/CoreText.h>
#import "LuaSyntaxHighlighter.h"
#import "UIColor+LuaHighlighting.h"

@interface LuaSyntaxHighlighter ()
@property (nonatomic, copy) NSMutableAttributedString *attributedText;

@property (nonatomic, strong) NSSet *keywords;
@property (nonatomic, strong) NSSet *controlFlow;
@property (nonatomic, strong) NSSet *valueKeywords;
@property (nonatomic, strong) NSSet *endable;

@end

@implementation LuaSyntaxHighlighter

- (id)init {
  self = [super init];
  if (self) {
    self.keywords = [NSSet setWithArray:@[@"function", @"require"]];
    self.controlFlow = [NSSet setWithArray:@[@"if", @"then", @"else", @"elseif",
                        @"do", @"in", @"for", @"and", @"or", @"not", @"until",
                        @"while", @"return", @"repeat", @"local"]];
    self.valueKeywords = [NSSet setWithArray:@[@"true", @"false", @"nil"]];
    self.endable = [NSSet setWithArray:@[@"if", @"do", @"function"]];
  }
  return self;
}

- (void)setText:(NSString *)text {
  _attributedText = [[NSMutableAttributedString alloc] initWithString:text
      attributes:@{NSForegroundColorAttributeName: [UIColor codeDefaultColor]}];
  
  [self highlightEntireString];
}

- (NSString *)text {
  return [self.attributedText string];
}

#pragma mark - Public

- (void)highlightEntireString {
  _attributedText = [[NSMutableAttributedString alloc] initWithAttributedString:
                     [self highlightString:[_attributedText string]]];
}

- (void)replaceThenRehighlightCharactersInRange:(NSRange)range
                                     withString:(NSString *)string {
  // Are we in a string literal?
  NSString *endcap = nil;
  BOOL inStringLiteral = [self locationInStringLiteral:range.location
                                            withEndcap:&endcap];
  if (inStringLiteral) {
    NSInteger endingEndcap;
    if ([self stringLiteralEndsInString:string
                             withEndcap:endcap
                             atLocation:&endingEndcap]) {
      NSAttributedString *toReplace =
          [[NSAttributedString alloc] initWithString:string
              attributes:@{NSForegroundColorAttributeName:
                             [UIColor codeStringColor]}];
      [self.attributedText replaceCharactersInRange:range
                               withAttributedString:toReplace];
      [self highlightEntireString];
      return;
    } else {
      NSAttributedString *toReplace =
          [[NSAttributedString alloc] initWithString:string
              attributes:@{NSForegroundColorAttributeName:
                             [UIColor codeStringColor]}];
      [self.attributedText replaceCharactersInRange:range
                               withAttributedString:toReplace];
      return;
    }
  }
  
  // Buffer left and right to find word boundaries
  NSString *before = [self.text substringToIndex:range.location];
  NSRange bufferedRange;
  NSCharacterSet *wordBoundary =
      [NSCharacterSet characterSetWithCharactersInString:
       @"\n\r\t (){}[]<>=~,./+-*;:"];
  NSRange lastBoundary =
      [before rangeOfCharacterFromSet:wordBoundary options:NSBackwardsSearch];
  if (lastBoundary.location == NSNotFound) {
    // We are at the beginning of the string.
    bufferedRange.location = 0;
  } else {
    bufferedRange.location = lastBoundary.location + lastBoundary.length;
    bufferedRange.length = range.length + (range.location - bufferedRange.location);
  }
  
  NSString *after =
      [self.text substringFromIndex:range.location + range.length];
  NSRange firstBoundary =
      [after rangeOfCharacterFromSet:wordBoundary];
  if (firstBoundary.location == NSNotFound) {
    // We are at the end of the string.
    bufferedRange.length = self.text.length - bufferedRange.location;
  } else {
    bufferedRange.length += firstBoundary.location;
  }
  
  range.location -= bufferedRange.location;
  NSString *substr = nil;
  if (bufferedRange.length > 0) {
    substr = [_attributedText.string substringWithRange:bufferedRange];
  }
  NSString *bufferedString = nil;
  NSAttributedString *attrStr = nil;
  if (substr && string) {
    bufferedString =
        [substr stringByReplacingCharactersInRange:range
                                        withString:string];
  } else if (substr) {
    bufferedString = substr;
  } else if (string) {
    bufferedString = string;
  }
  
  if (bufferedString) {
    NSArray *badWords = [[self.endable allObjects] arrayByAddingObject:@"end"];
    for (NSString *badWord in badWords) {
      NSRange checkRange = [bufferedString rangeOfString:badWord];
      if (checkRange.location != NSNotFound) {
        NSAttributedString *toReplace =
            [[NSAttributedString alloc] initWithString:bufferedString
                attributes:[self attributesWithColor:[UIColor codeDefaultColor]]];
        [self.attributedText replaceCharactersInRange:bufferedRange
                                 withAttributedString:toReplace];
        [self highlightEntireString];
        return;
      }
    }
    
    attrStr = [self highlightString:bufferedString];
  }
  
  [self.attributedText replaceCharactersInRange:bufferedRange
                           withAttributedString:attrStr];
}

#pragma mark - Private

- (NSDictionary *)attributesWithColor:(UIColor *)color {
  if (self.useCTAttributes) {
    return @{(id)kCTForegroundColorAttributeName: (id)color.CGColor};
  } else {
    return @{NSForegroundColorAttributeName: color};
  }
}

- (BOOL)locationInStringLiteral:(int)loc withEndcap:(NSString **)withEndcap {
  NSString *upTo = [self.text substringToIndex:loc];
  NSScanner *scanner = [NSScanner scannerWithString:upTo];
  scanner.charactersToBeSkipped = nil;
  NSCharacterSet *stringLiteral =
      [NSCharacterSet characterSetWithCharactersInString:@"\"'"];
  NSString *loopStr = nil;
  while (!scanner.isAtEnd) {
    if ([scanner scanCharactersFromSet:stringLiteral
                            intoString:&loopStr]) {
      NSString *endcap =
          loopStr.length > 1 ? [loopStr substringToIndex:1] : loopStr;
      NSString *endcapEscape = [NSString stringWithFormat:@"%@\\", endcap];
      NSCharacterSet *eeCharSet =
          [NSCharacterSet characterSetWithCharactersInString:endcapEscape];
      NSString *literalLoopStr = nil;
      BOOL gotEndcap = NO;
      while (!scanner.isAtEnd) {
        if ([scanner scanUpToCharactersFromSet:eeCharSet
                                    intoString:&literalLoopStr]) {
        } else if ([scanner scanString:@"\\"
                            intoString:&literalLoopStr]) {
        } else if ([scanner scanString:endcap
                            intoString:&literalLoopStr]) {
          gotEndcap = YES;
          break;
        }
      }
      if (!gotEndcap) {
        *withEndcap = endcap;
        return YES;
      }
    }
    [scanner scanUpToCharactersFromSet:stringLiteral intoString:nil];
  }
  return NO;
}

- (BOOL)stringLiteralEndsInString:(NSString *)string
                       withEndcap:(NSString *)endcap
                       atLocation:(NSInteger *)location {
  NSScanner *scanner = [NSScanner scannerWithString:string];
  NSString *endcapEscape = [NSString stringWithFormat:@"%@\\", endcap];
  NSCharacterSet *eeCharSet =
      [NSCharacterSet characterSetWithCharactersInString:endcapEscape];
  NSString *literalLoopStr = nil;
  while (!scanner.isAtEnd) {
    if ([scanner scanUpToCharactersFromSet:eeCharSet
                                intoString:&literalLoopStr]) {
    } else if ([scanner scanString:@"\\"
                        intoString:&literalLoopStr]) {
    } else if ([scanner scanString:endcap
                        intoString:&literalLoopStr]) {
      *location = scanner.scanLocation - 1;
      return YES;
    }
  }
  return NO;
}

#pragma mark - Highlighting

- (NSAttributedString *)highlightString:(NSString *)toHighlight {
  NSMutableAttributedString *highlighted =
      [[NSMutableAttributedString alloc] init];
  NSCharacterSet *newline =
      [NSCharacterSet characterSetWithCharactersInString:@"\n\r"];
  NSCharacterSet *wordBoundary =
      [NSCharacterSet characterSetWithCharactersInString:
       @"\n\r\t (){}[]<>=~,./+-*;:"];
  NSCharacterSet *whitespace =
      [NSCharacterSet whitespaceAndNewlineCharacterSet];
  NSCharacterSet *stringLiteral =
      [NSCharacterSet characterSetWithCharactersInString:@"\"'"];
  NSCharacterSet *numbers =
      [NSCharacterSet characterSetWithCharactersInString:@"1234567890"];
  
  NSScanner *scanner = [NSScanner scannerWithString:toHighlight];
  scanner.charactersToBeSkipped = nil;
  
  NSMutableArray *endStack = [NSMutableArray array];
  
  while (!scanner.isAtEnd) {
    NSString *loopStr = nil;
    
#pragma mark String Literal
    
    if ([scanner scanCharactersFromSet:stringLiteral
                                   intoString:&loopStr]) {
      NSMutableString *strLiteral = [loopStr mutableCopy];
      NSString *endcap =
          loopStr.length > 1 ? [loopStr substringToIndex:1] : loopStr;
      NSString *endcapEscape = [NSString stringWithFormat:@"%@\\", endcap];
      NSCharacterSet *eeCharSet =
          [NSCharacterSet characterSetWithCharactersInString:endcapEscape];
      NSString *literalLoopStr = nil;
      while (!scanner.isAtEnd) {
        if ([scanner scanUpToCharactersFromSet:eeCharSet
                                    intoString:&literalLoopStr]) {
          [strLiteral appendString:literalLoopStr];
        } else if ([scanner scanString:@"\\"
                            intoString:&literalLoopStr]) {
          [strLiteral appendString:literalLoopStr];
          NSRange escaped = NSMakeRange(scanner.scanLocation, 1);
          [strLiteral appendString:[scanner.string substringWithRange:escaped]];
          scanner.scanLocation++;
        } else if ([scanner scanString:endcap
                            intoString:&literalLoopStr]) {
          [strLiteral appendString:literalLoopStr];
          break;
        }
      }
      NSDictionary *attrs =
          [self attributesWithColor:[UIColor codeStringColor]];
      NSAttributedString *toAppend =
          [[NSAttributedString alloc] initWithString:strLiteral
                                          attributes:attrs];
      [highlighted appendAttributedString:toAppend];
      
#pragma mark Number Literal
      
    } else if ([scanner scanCharactersFromSet:numbers intoString:&loopStr]) {
      NSMutableString *numberLiteral = [loopStr mutableCopy];
      BOOL needsDecimal = NO;
      if ([scanner scanString:@"." intoString:nil]) {
        NSString *decimal = nil;
        if ([scanner scanCharactersFromSet:numbers intoString:&decimal]) {
          [numberLiteral appendFormat:@".%@", decimal];
        } else {
          needsDecimal = YES;
        }
      }
      
      NSDictionary *attrs =
          @{NSForegroundColorAttributeName: [UIColor codeNumberColor]};
      NSAttributedString *toAppend =
          [[NSAttributedString alloc] initWithString:numberLiteral
                                          attributes:attrs];
      [highlighted appendAttributedString:toAppend];
      
      if (needsDecimal) {
        NSDictionary *attrs =
            [self attributesWithColor:[UIColor codeDefaultColor]];
        NSAttributedString *toAppend =
            [[NSAttributedString alloc] initWithString:@"." attributes:attrs];
        [highlighted appendAttributedString:toAppend];
      }
      
#pragma mark Comment
      
    } else if ([scanner scanString:@"--" intoString:&loopStr]) {
      NSMutableString *comment = [loopStr mutableCopy];
      NSString *c = nil;
      [scanner scanUpToCharactersFromSet:newline intoString:&c];
      [comment appendString:c];
      NSDictionary *attrs =
          @{NSForegroundColorAttributeName: [UIColor codeCommentColor]};
      NSAttributedString *toAppend =
          [[NSAttributedString alloc] initWithString:comment attributes:attrs];
      [highlighted appendAttributedString:toAppend];
      
#pragma mark Whitespace
      
    } else if ([scanner scanCharactersFromSet:whitespace
                            intoString:&loopStr]) {
      NSAttributedString *toAppend =
          [[NSAttributedString alloc] initWithString:loopStr];
      [highlighted appendAttributedString:toAppend];
      
#pragma mark Word
      
    } else if ([scanner scanUpToCharactersFromSet:wordBoundary
                                       intoString:&loopStr]) {
      NSAttributedString *toAppend = nil;
      if ([loopStr isEqualToString:@"end"]) {
        NSAttributedString *matching = [endStack lastObject];
        NSRange effective;
        NSDictionary *attrs = [matching attributesAtIndex:0
                                           effectiveRange:&effective];
        toAppend = [[NSAttributedString alloc] initWithString:loopStr
                                                   attributes:attrs];
        [endStack removeLastObject];
      } else if ([self.endable containsObject:loopStr]) {
        toAppend = [self highlightWord:loopStr];
        [endStack addObject:toAppend];
      } else {
        toAppend = [self highlightWord:loopStr];
      }
      
      [highlighted appendAttributedString:toAppend];
      
#pragma mark Etc
      
    } else if ([scanner scanCharactersFromSet:wordBoundary
                                   intoString:&loopStr]) {
      NSDictionary *attrs =
          @{NSForegroundColorAttributeName: [UIColor codeDefaultColor]};
      NSAttributedString *toAppend =
          [[NSAttributedString alloc] initWithString:loopStr
                                          attributes:attrs];
      [highlighted appendAttributedString:toAppend];
    }
  }
  
  return highlighted;
}

- (NSAttributedString *)highlightWord:(NSString *)string {
  NSDictionary *attrs =
      [self attributesWithColor:[UIColor codeDefaultColor]];
  
#pragma mark Keyword
  
  if ([self.keywords containsObject:string]) {
    attrs =
        [self attributesWithColor:[UIColor codeKeywordColor]];
    
#pragma mark Control Flow
    
  } else if ([self.controlFlow containsObject:string]) {
    attrs =
        [self attributesWithColor:[UIColor codeControlFlowColor]];
    
#pragma mark Value Words
    
  } else if ([self.valueKeywords containsObject:string]) {
    attrs =
        [self attributesWithColor:[UIColor codeBooleanColor]];
    
  }
  NSAttributedString *attr = [[NSAttributedString alloc] initWithString:string
                                                             attributes:attrs];
  return attr;
}

@end
