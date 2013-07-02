//
//  LuaSyntaxStyler.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/1/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface LuaSyntaxStyler : NSObject

@property (nonatomic, strong) NSString *text;
@property (nonatomic, assign) NSInteger lastIndentJump;

- (void)replaceThenRestyleCharactersInRange:(NSRange)range
                                 withString:(NSString *)string
                                styledRange:(NSRange *)styledRange
                               styledString:(NSString **)styledString;

@end
