//
//  SDKScriptTabModel.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/28/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Foundation/Foundation.h>

@class SDKScriptEditorView;

@interface SDKScriptTabModel : NSObject

@property (strong) NSImage *largeImage;
@property (strong) NSImage *icon;
@property (strong) NSString *iconName;

@property (assign) BOOL isProcessing;
@property (assign) NSInteger objectCount;
@property (assign) BOOL isEdited;
@property (assign) BOOL isBlank;

@property (nonatomic, strong) SDKScriptEditorView *scriptEditor;

// designated initializer
- (id)init;

@end

