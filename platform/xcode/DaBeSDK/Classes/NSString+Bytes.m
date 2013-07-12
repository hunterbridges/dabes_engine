//
//  NSString+Bytes.m
//  DaBeSDK
//
//  Created by Hunter Bridges on 7/11/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "NSString+Bytes.h"

//
//  NSString+Bytes.m
//  CocoaSmugglr
//
//  Created by Kennard Smith on 5/14/09.
//  Copyright 2009 antisocialsoft. All rights reserved.
//

#import "NSString+Bytes.h"

#define KSByteSize 1024.

@implementation  NSString (Bytes)

+ (NSString *)stringFromBytes:(unsigned long long)size
{
	if (size == 0.) {
		return @"0 b";
  } else if (size > 0. && size < KSByteSize) {
    return [NSString stringWithFormat:@"%qu b", size];
  } else if (size >= KSByteSize && size < pow(KSByteSize, 2.)) {
    return [NSString stringWithFormat:@"%.0f kb", (size / KSByteSize)];
  } else if (size >= pow(KSByteSize, 2.) && size < pow(KSByteSize, 3.)) {
    return [NSString stringWithFormat:@"%.0f Mb", (size / pow(KSByteSize, 2.))];
  } else if (size >= pow(KSByteSize, 3.)) {
    return [NSString stringWithFormat:@"%.1f Gb", (size / pow(KSByteSize, 3.))];
  }
	
	return @"Unknown";
}

@end