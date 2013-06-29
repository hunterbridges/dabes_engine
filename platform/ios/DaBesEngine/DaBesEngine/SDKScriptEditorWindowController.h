//
//  SDKScriptEditorWindowController.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/28/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <PSMTabBarControl/PSMTabBarControl.h>

@class SDKEngineViewController;
@interface SDKScriptEditorWindowController : NSWindowController
    <PSMTabBarControlDelegate, NSOutlineViewDataSource, NSOutlineViewDelegate>

@property (nonatomic, assign) IBOutlet PSMTabBarControl *tab;
@property (nonatomic, weak) SDKEngineViewController *engineVC;
@property (nonatomic, assign) IBOutlet NSDrawer *treeDrawer;
@property (nonatomic, assign) IBOutlet NSOutlineView *treeView;

@end
