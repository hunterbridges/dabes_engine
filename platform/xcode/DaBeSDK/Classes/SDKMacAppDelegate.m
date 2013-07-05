//
//  AppDelegate.m
//  DaBeSDK
//
//  Created by Hunter Bridges on 6/17/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <DaBes-Mac/DaBes-Mac.h>
#import "SDKMacAppDelegate.h"
#import "SDKScriptEditorWindowController.h"
#import "SDKScriptEditorView.h"
#import "SDKScriptTabModel.h"
#import "SDKProjectEngineViewController.h"

@interface SDKMacAppDelegate () <NSWindowDelegate>

@property (nonatomic, strong) SDKProjectEngineViewController *engineVC;

@end

@implementation SDKMacAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  // Insert code here to initialize your application
  self.engineVC = [[SDKProjectEngineViewController alloc] init];
  self.engineVC.view.frame = ((NSView *)self.window.contentView).bounds;
  [self.engineVC.view setAutoresizingMask:(NSViewWidthSizable | NSViewHeightSizable)];
  [self.window.contentView addSubview:self.engineVC.view];
  
  // wtf
  self.inspectorView.touchInput = self.engineVC.touchInput;
  self.inspectorView.engineVC = self.engineVC;
  
  [self setUpMenu];
  
  [[NSNotificationCenter defaultCenter]
     addObserver:self
     selector:@selector(handleNewScene:)
     name:kNewSceneNotification
     object:self.engineVC];
}

- (void)setUpMenu {
  [self.inspectorItem setEnabled:YES];
}

- (IBAction)inspectorItemClicked:(id)sender {
  NSMenuItem *item = sender;
  if (item.state == NSOnState) {
    [self.inspectorWindow close];
  } else {
    [self.inspectorWindow makeKeyAndOrderFront:nil];
  }
}

- (void)handleNewScene:(NSNotification *)notification {
  if (self.engineVC.scene)
    [self setSceneRenderMode:self.engineVC.scene->render_mode];
}

- (void)setSceneRenderMode:(SceneRenderMode)renderMode {
  if (renderMode == kSceneRenderModeNormal) {
    self.renderNormalItem.state = NSOnState;
    self.renderPhysicsItem.state = NSOffState;
  } else if (renderMode == kSceneRenderModePhysicsDebug) {
    self.renderNormalItem.state = NSOffState;
    self.renderPhysicsItem.state = NSOnState;
  }
  self.engineVC.scene->render_mode = renderMode;
}

#pragma mark - IBActions

- (IBAction)scriptEditorItemClicked:(id)sender {
  NSMenuItem *item = sender;
  if (item.state == NSOnState) {
    [self.scriptEditorController.window close];
    self.scriptEditorController = nil;
    item.state = NSOffState;
  } else {
    self.scriptEditorController =
        [[SDKScriptEditorWindowController alloc] initWithWindowNibName:@"ScriptEditor"];
    self.scriptEditorController.engineVC = self.engineVC;
    [self.scriptEditorController.window makeKeyAndOrderFront:nil];
    item.state = NSOnState;
  }
}

- (IBAction)restartSceneItemClicked:(id)sender {
  [self.engineVC restartCurrentScene];
}

- (IBAction)restartGameClicked:(id)sender {
  [self.engineVC reboot];
}

- (IBAction)renderNormalItemClicked:(id)sender {
  [self setSceneRenderMode:kSceneRenderModeNormal];
}

- (IBAction)renderPhysicsItemClicked:(id)sender {
  [self setSceneRenderMode:kSceneRenderModePhysicsDebug];
}

- (IBAction)saveItemClicked:(id)sender {
  [self.scriptEditorController saveCurrentTab];
}

- (IBAction)revertItemClicked:(id)sender {
  [self.scriptEditorController revertCurrentTab];
}

- (IBAction)closeItemClicked:(id)sender {
  NSWindow *keyWindow = [NSApp keyWindow];
  id firstResponder = [keyWindow firstResponder];
  if (keyWindow && keyWindow == self.scriptEditorController.window) {
    [self.scriptEditorController closeItem:sender];
    return;
  }
  [firstResponder performClose:sender];
}

- (IBAction)createNewTabItemClicked:(id)sender {
  [self.scriptEditorController newTab];
}

- (IBAction)prevTabItemClicked:(id)sender {
  [self.scriptEditorController prevTab];
}

- (IBAction)nextTabItemClicked:(id)sender {
  [self.scriptEditorController nextTab];
}

#pragma mark - Window Delegate

- (void)windowWillClose:(NSNotification *)notification {
  if (notification.object == self.inspectorWindow) {
    self.inspectorItem.state = NSOffState;
  } else if (notification.object == self.scriptEditorController.window) {
    self.scriptEditorItem.state = NSOffState;
  }
}

- (void)windowDidBecomeKey:(NSNotification *)notification {
  if (notification.object == self.inspectorWindow) {
    self.inspectorItem.state = NSOnState;
  } else if (notification.object == self.scriptEditorController.window) {
    self.scriptEditorItem.state = NSOnState;
  }
}

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem {
  if (menuItem == self.saveItem) {
    return ([NSApp keyWindow] == self.scriptEditorController.window &&
            self.scriptEditorController.currentTabModel.isEdited);
  } else if (menuItem == self.revertItem) {
    return ([NSApp keyWindow] == self.scriptEditorController.window &&
            self.scriptEditorController.currentTabModel.isEdited &&
            self.scriptEditorController.currentTabModel.scriptEditor.path);
  } else if (menuItem == self.prevTabItem || menuItem == self.nextTabItem) {
    return ([NSApp keyWindow] == self.scriptEditorController.window &&
            self.scriptEditorController.tabCount > 1);
  } else if (menuItem == self.createNewTabItem) {
    return ([NSApp keyWindow] == self.scriptEditorController.window);
  }
  return menuItem.action != 0;
}

@end
