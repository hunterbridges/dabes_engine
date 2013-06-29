//
//  AppDelegate.m
//  DaBeSDK
//
//  Created by Hunter Bridges on 6/17/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "SDKAppDelegate.h"
#import "SDKEngineViewController.h"
#import "SDKScriptManagerView.h"
#import "SDKScriptEditorWindowController.h"

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
@property (nonatomic, strong) SDKScriptManagerView *scriptManagerView;

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

@end
