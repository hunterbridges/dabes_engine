//
//  SDKScriptTabModel.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/28/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "SDKScriptTabModel.h"

@implementation SDKScriptTabModel

- (id)init {
	if((self = [super init])) {
		_isProcessing = NO;
		_icon = nil;
		_iconName = nil;
    _largeImage = nil;
		_objectCount = 0;
		_isEdited = NO;
	}
	return self;
}

@end
