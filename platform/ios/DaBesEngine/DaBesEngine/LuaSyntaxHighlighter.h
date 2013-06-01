//
//  LuaSyntaxHighlighter.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface LuaSyntaxHighlighter : NSObject

@property (nonatomic, readonly, copy) NSMutableAttributedString *attributedText;
@property (nonatomic, strong) NSString *text;

- (void)highlightEntireString;
- (void)replaceThenRehighlightCharactersInRange:(NSRange)range
                                     withString:(NSString *)string;
  
@end
