//
//  ScriptBrowserViewController.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/3/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "DirBrowserViewController.h"

@interface DirBrowserViewController ()

@property (nonatomic, copy) NSString *path;
@property (nonatomic, copy)
    void(^selectionBlock)(DirBrowserViewController *, NSString *);
@property (nonatomic, strong) NSArray *items;
@property (nonatomic, strong) NSFileManager *fileManager;

@end

@implementation DirBrowserViewController

- (id)initWithPath:(NSString *)path
    withFileSelectionBlock:(void(^)(DirBrowserViewController *, NSString *))selection {
  self = [super initWithStyle:UITableViewStylePlain];
  if (self) {
    self.selectionBlock = selection;
    self.path = path;
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];

  self.navigationItem.title = [self.path lastPathComponent];
    
  NSError *error = nil;
  self.fileManager = [[NSFileManager alloc] init];
  self.items = [self.fileManager contentsOfDirectoryAtPath:self.path error:&error];
  if (error) {
    NSLog(@"Error: %@", error.description);
  }
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
  // Dispose of any resources that can be recreated.
}

#pragma mark - Table view data source

- (NSInteger)numberOfSectionsInTableView:(UITableView *)tableView {
  return 1;
}

- (NSInteger)tableView:(UITableView *)tableView
   numberOfRowsInSection:(NSInteger)section {
  return self.items.count;
}

- (UITableViewCell *)tableView:(UITableView *)tableView
         cellForRowAtIndexPath:(NSIndexPath *)indexPath {
  static NSString *CellIdentifier = @"Cell";
  UITableViewCell *cell = [tableView dequeueReusableCellWithIdentifier:CellIdentifier];
  
  if (!cell) {
    cell = [[UITableViewCell alloc] initWithStyle:UITableViewCellStyleDefault
                                  reuseIdentifier:CellIdentifier];
  }
  
  cell.textLabel.text = [self.items[indexPath.row] lastPathComponent];
  
  return cell;
}

#pragma mark - Table view delegate

- (void)tableView:(UITableView *)tableView
    didSelectRowAtIndexPath:(NSIndexPath *)indexPath {
  BOOL isDir;
  NSString *nextPath =
      [self.path stringByAppendingPathComponent:self.items[indexPath.row]];
  if ([self.fileManager fileExistsAtPath:nextPath isDirectory:&isDir]) {
    if (isDir) {
      DirBrowserViewController *next =
          [[DirBrowserViewController alloc] initWithPath:nextPath
                                  withFileSelectionBlock:self.selectionBlock];
      [self.navigationController pushViewController:next animated:YES];
    } else {
      self.selectionBlock(self, nextPath);
    }
  }
}

@end
