//
//  ScriptEditorViewController.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 5/31/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <DaBes-iOS/DaBes-iOS.h>
#import "CodeEditorView.h"

@class SDKiOSProjectEngineViewController;
@interface ScriptEditorViewController : UIViewController

- (id)initWithEngineVC:(SDKiOSProjectEngineViewController *)engineVC
              withPath:(NSString *)path;

@property (nonatomic, readonly) CodeEditorView *editorView;
@property (nonatomic, copy) NSString *queuedScript;

@end
