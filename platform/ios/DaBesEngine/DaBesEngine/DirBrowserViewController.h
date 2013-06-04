//
//  ScriptBrowserViewController.h
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/3/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <UIKit/UIKit.h>

@interface DirBrowserViewController : UITableViewController

@property (nonatomic, copy, readonly)
    void(^selectionBlock)(DirBrowserViewController *, NSString *);

- (id)initWithPath:(NSString *)path
  withFileSelectionBlock:(void(^)(DirBrowserViewController *, NSString *))selection;

@end
