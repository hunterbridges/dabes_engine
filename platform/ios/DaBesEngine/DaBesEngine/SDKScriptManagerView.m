//
//  SDKScriptManagerView.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/26/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <PSMTabBarControl/PSMTabBarControl.h>
#import "SDKScriptManagerView.h"
#import "SDKScriptEditorView.h"

@interface SDKScriptManagerView () <NSToolbarDelegate>

@property (nonatomic, strong) SDKScriptEditorView *scriptEditor;
@property (nonatomic, strong) PSMTabBarControl *tabBarControl;

@end

@implementation SDKScriptManagerView

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
    }
    
    return self;
}

- (void)awakeFromNib {
  self.scriptEditor = [[SDKScriptEditorView alloc] initWithFrame:self.bounds];
  self.scriptEditor.autoresizingMask =
      (NSViewWidthSizable | NSViewHeightSizable);
  self.scriptEditor.scriptManager = self;
  [self addSubview:self.scriptEditor];
}

#pragma mark - Toolbar Delegate


@end
