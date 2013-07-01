//
//  ScriptEditorViewController.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "engine.h"
#import "CodeEditorView.h"

@interface ScriptEditorViewController : UIViewController

- (id)initWithEngineVC:(EngineViewController *)engineVC
              withPath:(NSString *)path;

@property (nonatomic, readonly) CodeEditorView *editorView;
@property (nonatomic, copy) NSString *queuedScript;

@end
