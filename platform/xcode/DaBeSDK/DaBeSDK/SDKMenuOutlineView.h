//
//  SDKDirTreeOutlineView.h
//  DaBeSDK
//
//  Created by Hunter Bridges on 7/5/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@class SDKMenuOutlineView;
@protocol SDKMenuOutlineViewDelegate <NSOutlineViewDelegate>

- (NSMenu *)outlineView:(SDKMenuOutlineView *)outlineView
            menuForItem:(id)item;

@end

@interface SDKMenuOutlineView : NSOutlineView

@property (nonatomic, weak) id<SDKMenuOutlineViewDelegate> delegate;

- (NSMenu *)defaultMenuForItem:(id)item;

@end
