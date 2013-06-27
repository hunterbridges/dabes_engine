//
//  SDKScriptEditorView.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/26/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SDKScriptManagerView.h"

@interface SDKScriptEditorView : NSView

@property (nonatomic, copy) NSString *syntaxDefinition;
@property (nonatomic, weak) SDKScriptManagerView *scriptManager;

@end
