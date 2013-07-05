//
//  SDKScriptEditorWindowController.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/28/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <PSMTabBarControl/PSMTabBarControl.h>
#import <PSMTabBarControl/PSMRolloverButton.h>
#import <DaBes-Mac/DABProjectManager.h>
#import "FileSystemItem.h"
#import "SDKMacAppDelegate.h"
#import "SDKScriptEditorWindowController.h"
#import "SDKScriptEditorView.h"
#import "SDKScriptTabModel.h"

@interface SDKScriptEditorWindowController () <NSTabViewDelegate,
    NSWindowDelegate, NSAlertDelegate>

@property (nonatomic, strong) NSTabView *tabView;
@property (nonatomic, assign) IBOutlet NSView *contentContainerView;
@property (nonatomic, strong) FileSystemItem *rootFileSystemItem;

@end

@implementation SDKScriptEditorWindowController

- (void)awakeFromNib {
  [super awakeFromNib];
  
  self.window.delegate = self;
  
  self.tabView = [[NSTabView alloc] init];
  //self.tabView.frame = self.scriptManager.frame;
  //[self.window.contentView addSubview:self.tabView];
  
  [[self.tab addTabButton] setTarget:self];
  [[self.tab addTabButton] setAction:@selector(addNewTab:)];
  [self.tab setStyleNamed:@"Unified"];
  [self.tab setShowAddTabButton:YES];
  self.tab.tabView = self.tabView;
  self.tabView.delegate = (id<NSTabViewDelegate>)self.tab;
  
  [self addNewTab:self withPath:nil];
  
  [[NSNotificationCenter defaultCenter]
      addObserver:self
      selector:@selector(handleScriptError:)
      name:kEngineHasScriptErrorNotification
      object:nil];
  [[NSNotificationCenter defaultCenter]
      addObserver:self
      selector:@selector(handleScriptPanic:)
      name:kEngineHasScriptPanicNotification
      object:nil];
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (IBAction)addNewTab:(id)sender {
  [self addNewTab:sender withPath:nil];
}

- (NSTabViewItem *)addNewTab:(id)sender withPath:(NSString *)path {
  return [self addNewTab:sender withPath:path andSelect:YES];
}

- (NSTabViewItem *)addNewTab:(id)sender withPath:(NSString *)path andSelect:(BOOL)select {
	SDKScriptTabModel *newModel = [[SDKScriptTabModel alloc] init];
	NSTabViewItem *newItem = [(NSTabViewItem *)[NSTabViewItem alloc] initWithIdentifier:newModel];
  NSString *label = path ? [path lastPathComponent] : @"Untitled";
	[newItem setLabel:label];
  
  newModel.scriptEditor = [[SDKScriptEditorView alloc] initWithPath:path];
  newModel.isBlank = !path;
  newModel.scriptEditor.scriptManager = self;
  newModel.scriptEditor.tabModel = newModel;
	[self.tabView addTabViewItem:newItem];
  if (select) {
    [self.tabView selectTabViewItem:newItem]; // this is optional, but expected behavior
  }
  return newItem;
}

- (NSTabViewItem *)openPath:(NSString *)path andSelect:(BOOL)select {
  for (NSTabViewItem *tabItem in self.tabView.tabViewItems) {
    SDKScriptTabModel *tabModel = tabItem.identifier;
    if ([tabModel.scriptEditor.path isEqualToString:path]) {
      if (select) [self.tabView selectTabViewItem:tabItem];
      return tabItem;
    }
    
    if (tabModel.isBlank && !tabModel.isEdited) {
      tabModel.scriptEditor.path = path;
      tabModel.isBlank = NO;
      NSString *label = path ? [path lastPathComponent] : @"Untitled";
      [tabItem setLabel:label];
      if (select) {
        [self.tabView selectTabViewItem:tabItem];
      }
      return tabItem;
    }
  }
  
  return [self addNewTab:self withPath:path andSelect:select];
}

- (void)windowDidLoad
{
  [super windowDidLoad];
  
  // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
  [self.treeDrawer open];
  CGSize treeSize = self.treeDrawer.contentSize;
  treeSize.width = 300;
  self.treeDrawer.contentSize = treeSize;
  self.treeDrawer.contentView.autoresizingMask = (NSViewWidthSizable |
                                                  NSViewHeightSizable);
}

- (void)saveEditorToPath:(NSString *)path
             withTabItem:(NSTabViewItem *)tabViewItem {
  SDKScriptTabModel *tabModel = tabViewItem.identifier;
  NSFileManager *fileManager = [[NSFileManager alloc] init];
  NSData *fileData =
      [tabModel.scriptEditor.string dataUsingEncoding:NSUTF8StringEncoding];
  [fileManager createFileAtPath:path
                       contents:fileData
                     attributes:nil];
  tabModel.isEdited = NO;
}

- (NSTabViewItem *)currentTab {
  return [self.tabView selectedTabViewItem];
}

- (SDKScriptTabModel *)currentTabModel {
  return self.currentTab.identifier;
}

- (NSUInteger)tabCount {
  return self.tabView.tabViewItems.count;
}

- (void)newTab {
  [self addNewTab:self];
}

- (void)prevTab {
  NSInteger idx = [self.tabView.tabViewItems indexOfObject:self.currentTab];
  idx--;
  if (idx < 0) idx = self.tabView.tabViewItems.count - 1;
  [self.tabView selectTabViewItemAtIndex:idx];
}

- (void)nextTab {
  NSInteger idx = [self.tabView.tabViewItems indexOfObject:self.currentTab];
  idx++;
  if (idx >= self.tabView.tabViewItems.count) idx = 0;
  [self.tabView selectTabViewItemAtIndex:idx];
}

- (void)closeItem:(id)sender {
  if (self.tabView.tabViewItems.count > 1) {
    BOOL shouldClose =
        [self tabView:self.tabView shouldCloseTabViewItem:self.currentTab];
    if (shouldClose) {
      [self.tabView removeTabViewItem:self.currentTab];
    }
  } else {
    [self.window performClose:sender];
  }
}

- (void)saveCurrentTab {
  [self saveEditorWithTabItem:self.currentTab];
}

- (void)revertCurrentTab {
  self.currentTabModel.scriptEditor.path =
      self.currentTabModel.scriptEditor.path;
  [self.currentTabModel.scriptEditor reinject];
  self.currentTabModel.isEdited = NO;
}

- (void)saveEditorWithTabItem:(NSTabViewItem *)tabViewItem {
  [self saveEditorWithTabItem:tabViewItem andClose:NO];
}

- (void)saveEditorWithTabItem:(NSTabViewItem *)tabViewItem
                     andClose:(BOOL)andClose {
  SDKScriptTabModel *tabModel = tabViewItem.identifier;
  NSString *path = tabModel.scriptEditor.path;
  if (path) {
    [self saveEditorToPath:path withTabItem:tabViewItem];
    if (andClose) {
      if (self.tabView.tabViewItems.count > 1) {
        [self.tabView removeTabViewItem:tabViewItem];
      } else {
        [self.window close];
      }
    }
    return;
  }
  
  double delayInSeconds = 0.1;
  dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
  dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
    NSSavePanel *savePanel = [NSSavePanel savePanel];
    savePanel.allowedFileTypes = @[@"lua"];
    savePanel.directoryURL = [NSURL fileURLWithPath:self.rootFileSystemItem.fullPath isDirectory:YES];
    [savePanel beginSheetModalForWindow:self.window
        completionHandler:^(NSInteger result) {
          if (result) {
            [self saveEditorToPath:savePanel.URL.path withTabItem:tabViewItem];

            if (self.tabView.tabViewItems.count > 1) {
              [self.tabView removeTabViewItem:tabViewItem];
            } else {
              [self.window close];
            }
          }
        }];
  });
}

#pragma mark - Notif Handlers

- (void)handleScriptError:(NSNotification *)notif {
    NSString *string = notif.object;
    NSString *filename = nil;
    NSString *errMsg = nil;
    NSNumber *errorLine = [SDKScriptEditorView extractErrorLine:string
                                                       filename:&filename
                                                        message:&errMsg];
    SDKScriptTabModel *tabModel = nil;
    if ([[filename substringToIndex:1] isEqualToString:@"["]) {
        // Injected script
        NSTabViewItem *item = [self.tabView selectedTabViewItem];
        tabModel = item.identifier;
    } else {
        NSTabViewItem *item = [self openPath:filename andSelect:NO];
        tabModel = item.identifier;
    }
    [tabModel.scriptEditor setErrorLine:errorLine withMessage:errMsg];
}

- (void)handleScriptPanic:(NSNotification *)notif {
    NSString *string = notif.object;
    NSLog(@"Script panic -- %@", string);
}

#pragma mark - Window Delegate

- (BOOL)windowShouldClose:(id)sender {
  /*
   */
  for (NSTabViewItem *tabItem in self.tabView.tabViewItems) {
    [self.tabView selectTabViewItem:tabItem];
    BOOL shouldClose =
        [self tabView:self.tabView shouldCloseTabViewItem:tabItem];
    if (!shouldClose) return NO;
  }
  
  return YES;
}

- (void)windowWillClose:(NSNotification *)notification {
  [[NSApp delegate] windowWillClose:notification];
}
  
#pragma mark - Tab View Delegate

- (void)tabView:(NSTabView *)aTabView didSelectTabViewItem:(NSTabViewItem *)tabViewItem {
	// need to update bound values to match the selected tab
  if (![[tabViewItem identifier] isKindOfClass:[SDKScriptTabModel class]]) return;
  
  SDKScriptTabModel *tabModel = [tabViewItem identifier];
  [self.contentContainerView.subviews makeObjectsPerformSelector:@selector(removeFromSuperview)];
  if (tabModel.scriptEditor) {
    [self.contentContainerView addSubview:tabModel.scriptEditor];
    tabModel.scriptEditor.frame = self.contentContainerView.bounds;
    tabModel.scriptEditor.autoresizingMask = (NSViewHeightSizable |
                                              NSViewWidthSizable);
  }
}

- (BOOL)tabView:(NSTabView *)aTabView shouldCloseTabViewItem:(NSTabViewItem *)tabViewItem {
  if (![[tabViewItem identifier] isKindOfClass:[SDKScriptTabModel class]]) return YES;
  
  SDKScriptTabModel *tabModel = [tabViewItem identifier];
	if(tabModel.isEdited) {
    NSString *titleText = (tabModel.scriptEditor.path ?
                           [NSString stringWithFormat:@"Are you sure you want to close %@?",
                               [tabModel.scriptEditor.path lastPathComponent]] :
                           @"Are you sure you want to close Untitled?");
		NSAlert *editedAlert = [NSAlert alertWithMessageText:titleText
                                           defaultButton:@"Save and Close"
                                         alternateButton:@"Cancel"
                                             otherButton:@"Close Without Saving"
                               informativeTextWithFormat:@"This file has been changed."];
		[editedAlert beginSheetModalForWindow:self.window
                            modalDelegate:self
                           didEndSelector:@selector(editedAlertDidEnd:returnCode:contextInfo:)
                              contextInfo:(void *)tabViewItem];
		return NO;
	}
	return YES;
}


- (BOOL)tabView:(NSTabView*)aTabView shouldDragTabViewItem:(NSTabViewItem *)tabViewItem fromTabBar:(PSMTabBarControl *)tabBarControl {
	return YES;
}

- (BOOL)tabView:(NSTabView*)aTabView shouldDropTabViewItem:(NSTabViewItem *)tabViewItem inTabBar:(PSMTabBarControl *)tabBarControl {
	return YES;
}

#pragma mark - Alert Delegate

- (void)editedAlertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo {
  switch (returnCode) {
    case 1: {
      // Save and Close
      NSTabViewItem *tabViewItem = (__bridge NSTabViewItem *)contextInfo;
      [self saveEditorWithTabItem:tabViewItem andClose:YES];
    } break;
      
    case -1:
      // Close without saving
      if (contextInfo) {
        NSTabViewItem *tabViewItem = (__bridge NSTabViewItem *)contextInfo;

        if (self.tabView.tabViewItems.count > 1) {
          [self.tabView removeTabViewItem:tabViewItem];
        } else {
          [self.window close];
        }
      }
      
      break;
      
    default:
      // Cancel
      break;
  }
}

#pragma mark - Outline View Data Source

- (FileSystemItem *)rootFileSystemItem {
  if (_rootFileSystemItem) return _rootFileSystemItem;
  
  NSString *path = [[DABProjectManager sharedInstance].projectDir stringByAppendingPathComponent:@"scripts"];
  _rootFileSystemItem = [[FileSystemItem alloc] initWithPath:path
                                                      parent:nil];
  return _rootFileSystemItem;
}

- (NSInteger)outlineView:(NSOutlineView *)outlineView numberOfChildrenOfItem:(id)item {
  return (item == nil) ? 1 : [item numberOfChildren];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView isItemExpandable:(id)item {
  return (item == nil) ? YES : ([item numberOfChildren] != -1);
}

- (id)outlineView:(NSOutlineView *)outlineView child:(NSInteger)index ofItem:(id)item {
  
  return (item == nil) ? [self rootFileSystemItem] : [(FileSystemItem *)item childAtIndex:index];
}

- (id)outlineView:(NSOutlineView *)outlineView objectValueForTableColumn:(NSTableColumn *)tableColumn byItem:(id)item {
  return (item == nil) ? @"/" : [item relativePath];
}

- (BOOL)outlineView:(NSOutlineView *)outlineView shouldSelectItem:(id)item {
  if ([item numberOfChildren] == -1) {
    [self openPath:[item fullPath] andSelect:YES];
  }
  return YES;
}

@end