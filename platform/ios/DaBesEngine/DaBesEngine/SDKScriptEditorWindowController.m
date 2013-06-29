//
//  SDKScriptEditorWindowController.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/28/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <PSMTabBarControl/PSMTabBarControl.h>
#import <PSMTabBarControl/PSMRolloverButton.h>
#import "FileSystemItem.h"
#import "SDKAppDelegate.h"
#import "SDKScriptEditorWindowController.h"
#import "SDKScriptEditorView.h"
#import "SDKScriptTabModel.h"

@interface SDKScriptEditorWindowController () <NSTabViewDelegate,
    NSWindowDelegate>

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
}

- (IBAction)addNewTab:(id)sender {
  [self addNewTab:sender withPath:nil];
}

- (void)addNewTab:(id)sender withPath:(NSString *)path {
	SDKScriptTabModel *newModel = [[SDKScriptTabModel alloc] init];
	NSTabViewItem *newItem = [(NSTabViewItem *)[NSTabViewItem alloc] initWithIdentifier:newModel];
  NSString *label = path ? [path lastPathComponent] : @"Untitled";
	[newItem setLabel:label];
  
  newModel.scriptEditor = [[SDKScriptEditorView alloc] initWithPath:path];
  newModel.isBlank = !path;
  newModel.scriptEditor.scriptManager = self;
  newModel.scriptEditor.tabModel = newModel;
	[self.tabView addTabViewItem:newItem];
	[self.tabView selectTabViewItem:newItem]; // this is optional, but expected behavior
}

- (void)openPath:(NSString *)path {
  for (NSTabViewItem *tabItem in self.tabView.tabViewItems) {
    SDKScriptTabModel *tabModel = tabItem.identifier;
    if ([tabModel.scriptEditor.path isEqualToString:path]) {
      [self.tabView selectTabViewItem:tabItem];
      return;
    }
    
    if (tabModel.isBlank && !tabModel.isEdited) {
      tabModel.scriptEditor.path = path;
      tabModel.isBlank = NO;
      NSString *label = path ? [path lastPathComponent] : @"Untitled";
      [tabItem setLabel:label];
      [self.tabView selectTabViewItem:tabItem];
      return;
    }
  }
  
  [self addNewTab:self withPath:path];
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

#pragma mark - Window Delegate

- (BOOL)windowShouldClose:(id)sender {
  for (NSTabViewItem *tabItem in self.tabView.tabViewItems) {
    [self.tabView selectTabViewItem:tabItem];
    BOOL shouldClose =
        [self tabView:self.tabView shouldCloseTabViewItem:tabItem];
    if (!shouldClose) return NO;
  }
  
  return YES;
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
    NSString *titleText = [NSString stringWithFormat:@"Are you sure you want to close `%@`?",
                                                    [tabModel.scriptEditor.path lastPathComponent]];
		NSAlert *editedAlert = [NSAlert alertWithMessageText:titleText
                                           defaultButton:@"Save and Close"
                                         alternateButton:@"Cancel"
                                             otherButton:@"Close Without Saving"
                               informativeTextWithFormat:@"File has been changed."];
		[editedAlert beginSheetModalForWindow:self.window
                            modalDelegate:nil
                           didEndSelector:nil
                              contextInfo:nil];
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

#pragma mark - Outline View Data Source

- (FileSystemItem *)rootFileSystemItem {
  if (_rootFileSystemItem) return _rootFileSystemItem;
  
  _rootFileSystemItem = [[FileSystemItem alloc] initWithPath:@"/Users/hbridges/Coding/dabes_engine/scripts/"
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
    [self openPath:[item fullPath]];
  }
  return YES;
}

@end
