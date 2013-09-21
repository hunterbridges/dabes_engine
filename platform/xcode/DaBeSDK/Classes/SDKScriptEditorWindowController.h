//
//  SDKScriptEditorWindowController.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/28/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <PSMTabBarControl/PSMTabBarControl.h>
#import "SDKMenuOutlineView.h"
#import "SDKConsolePanel.h"

@class DABMacEngineViewController;
@class SDKScriptTabModel;
@interface SDKScriptEditorWindowController : NSWindowController
    <PSMTabBarControlDelegate, NSOutlineViewDataSource, SDKMenuOutlineViewDelegate>

@property (nonatomic, assign) IBOutlet PSMTabBarControl *tab;
@property (nonatomic, weak) DABMacEngineViewController *engineVC;
@property (nonatomic, assign) IBOutlet NSDrawer *treeDrawer;
@property (nonatomic, assign) IBOutlet SDKMenuOutlineView *treeView;

@property (nonatomic, weak) IBOutlet SDKConsolePanel *consolePanel;

@property (nonatomic, assign, readonly) NSUInteger tabCount;
@property (nonatomic, strong, readonly) NSTabViewItem *currentTab;
@property (nonatomic, strong, readonly) SDKScriptTabModel *currentTabModel;

- (void)newTab;
- (void)prevTab;
- (void)nextTab;
- (void)saveCurrentTab;
- (void)revertCurrentTab;
- (void)closeItem:(id)sender;
  
@end
