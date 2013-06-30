//
//  AppDelegate.m
//  DaBeSDK
//
//  Created by Hunter Bridges on 6/17/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "SDKAppDelegate.h"
#import "SDKEngineViewController.h"
#import "SDKScriptEditorWindowController.h"
#import "SDKScriptEditorView.h"
#import "SDKScriptTabModel.h"
#import "scene.h"

const char *resource_path(const char *filename) {
  NSString *nsFilename = [NSString stringWithCString:filename
                                            encoding:NSUTF8StringEncoding];
  
  NSFileManager *manager = [NSFileManager defaultManager];
  NSString *bundlePath = [[[NSBundle mainBundle] resourcePath]
                            stringByAppendingPathComponent:nsFilename];
  NSArray *docsPath =
      NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                          NSUserDomainMask, YES);
  NSString *docsFile = [[docsPath objectAtIndex:0]
                       stringByAppendingPathComponent:nsFilename];
  
  const char *cFullpath;
  if ([manager fileExistsAtPath:docsFile]) {
    cFullpath = [docsFile cStringUsingEncoding:NSUTF8StringEncoding];
  } else if ([manager fileExistsAtPath:bundlePath]) {
    cFullpath = [bundlePath cStringUsingEncoding:NSUTF8StringEncoding];
  } else {
    cFullpath = [bundlePath cStringUsingEncoding:NSUTF8StringEncoding];
  }
  
  return cFullpath;
}

FILE *load_resource(char *filename) {
    const char *cFullpath = resource_path(filename);
    FILE *file = fopen(cFullpath, "r");
    return file;
}

@interface SDKAppDelegate () <NSWindowDelegate>

@property (nonatomic, strong) SDKEngineViewController *engineVC;

@end

@implementation SDKAppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
  // Insert code here to initialize your application
  self.engineVC = [[SDKEngineViewController alloc] init];
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
