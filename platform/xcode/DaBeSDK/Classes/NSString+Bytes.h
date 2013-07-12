//
//  NSString+Bytes.h
//  DaBeSDK
//
//  Created by Hunter Bridges on 7/11/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSString (Bytes)

+ (NSString *)stringFromBytes:(unsigned long long)size;

@end
