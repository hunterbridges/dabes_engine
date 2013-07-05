//
//  AppDelegate.h
//  DaBeSDK
//
//  Created by Hunter Bridges on 6/17/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "SDKInspectorView.h"

@class SDKScriptEditorWindowController;
@interface SDKMacAppDelegate : NSObject <NSApplicationDelegate>

@property (assign) IBOutlet NSWindow *welcomeWindow;
@property (assign) IBOutlet NSWindow *engineWindow;
@property (assign) IBOutlet NSWindow *inspectorWindow;

@property (nonatomic, copy) NSString *curentProject;
@property (assign) IBOutlet NSMenuItem *cNewProjectItem;
@property (assign) IBOutlet NSMenuItem *openProjectItem;
@property (assign) IBOutlet NSMenuItem *closeProjectItem;

@property (assign) IBOutlet NSMenuItem *saveItem;
@property (assign) IBOutlet NSMenuItem *revertItem;

@property (assign) IBOutlet NSMenuItem *createNewTabItem;
@property (assign) IBOutlet NSMenuItem *prevTabItem;
@property (assign) IBOutlet NSMenuItem *nextTabItem;

@property (assign) IBOutlet NSMenuItem *inspectorItem;
@property (assign) IBOutlet NSMenuItem *scriptEditorItem;
@property (assign) IBOutlet SDKInspectorView *inspectorView;
@property (nonatomic, strong) SDKScriptEditorWindowController *scriptEditorController;

@property (assign) IBOutlet NSMenuItem *restartSceneItem;
@property (assign) IBOutlet NSMenuItem *restartGameItem;
@property (assign) IBOutlet NSMenuItem *renderModesItem;
@property (assign) IBOutlet NSMenuItem *renderNormalItem;
@property (assign) IBOutlet NSMenuItem *renderPhysicsItem;

@end
