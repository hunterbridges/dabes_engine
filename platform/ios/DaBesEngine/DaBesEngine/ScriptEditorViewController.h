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

- (id)initWithEngine:(Engine *)engine;
  
@property (nonatomic, readonly) CodeEditorView *editorView;

@end
