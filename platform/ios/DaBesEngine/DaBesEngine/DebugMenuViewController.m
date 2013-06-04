//
//  DebugMenuViewController.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/4/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "DebugMenuViewController.h"

#import "ConsoleView.h"
#import "EngineViewController.h"

static const CGFloat kHeight = 120.f;
static const int kPages = 4;

@interface DebugMenuViewController ()
  <UIScrollViewDelegate, UIImagePickerControllerDelegate,
      UINavigationControllerDelegate, UIAlertViewDelegate> {
    Input *_touchInput;
}

@property (nonatomic, strong) EngineViewController *engineVC;
@property (nonatomic, strong) UIScrollView *scrollView;

@property (nonatomic, strong) ConsoleView *console;
@property (nonatomic, strong) UIPopoverController *popover;
@property (nonatomic, strong) UISwitch *camDebugSwitch;
@property (nonatomic, strong) UISwitch *gridSwitch;
  
@property (nonatomic, assign) GraphicalResourceKind importKind;
@property (nonatomic, strong) UIImage *importImage;
@property (nonatomic, strong) UIAlertView *importAlert;
@end

@implementation DebugMenuViewController

- (id)initWithEngineVC:(EngineViewController *)engineVC
        withTouchInput:(Input *)touchInput {
  self = [super init];
  if (self) {
    self.engineVC = engineVC;
    _touchInput = touchInput;
  }
  return self;
}

- (void)loadView {
  self.scrollView = [[UIScrollView alloc] init];
  self.scrollView.autoresizingMask = UIViewAutoresizingFlexibleWidth;
  self.scrollView.pagingEnabled = YES;
  self.scrollView.scrollEnabled = YES;
  self.scrollView.translatesAutoresizingMaskIntoConstraints = YES;
  self.scrollView.userInteractionEnabled = YES;
  self.scrollView.delaysContentTouches = YES;
  self.scrollView.clipsToBounds = NO;
  self.scrollView.delegate = self;
  self.view = self.scrollView;
}

- (void)viewDidLayoutSubviews {
  [super viewWillLayoutSubviews];
}

- (void)viewDidLoad
{
  [super viewDidLoad];
  
  self.console = [[ConsoleView alloc] init];
  self.console.translatesAutoresizingMaskIntoConstraints = NO;
  [self.scrollView addSubview:self.console];
  
  UILabel *camLabel = [[UILabel alloc] init];
  camLabel.backgroundColor = [UIColor clearColor];
  camLabel.textColor = [UIColor whiteColor];
  camLabel.textAlignment = NSTextAlignmentLeft;
  camLabel.text = @"Camera";
  camLabel.font = [UIFont boldSystemFontOfSize:20];
  camLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.scrollView addSubview:camLabel];
  
  UILabel *envLabel = [[UILabel alloc] init];
  envLabel.backgroundColor = [UIColor clearColor];
  envLabel.textColor = [UIColor whiteColor];
  envLabel.textAlignment = NSTextAlignmentLeft;
  envLabel.text = @"Environment";
  envLabel.font = [UIFont boldSystemFontOfSize:20];
  envLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.scrollView addSubview:envLabel];
  
  UILabel *resLabel = [[UILabel alloc] init];
  resLabel.backgroundColor = [UIColor clearColor];
  resLabel.textColor = [UIColor whiteColor];
  resLabel.textAlignment = NSTextAlignmentLeft;
  resLabel.text = @"Resources";
  resLabel.font = [UIFont boldSystemFontOfSize:20];
  resLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.scrollView addSubview:resLabel];
  
  UILabel *rotLabel = [[UILabel alloc] init];
  rotLabel.backgroundColor = [UIColor clearColor];
  rotLabel.textColor = [UIColor whiteColor];
  rotLabel.textAlignment = NSTextAlignmentLeft;
  rotLabel.text = @"Rot";
  rotLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.scrollView addSubview:rotLabel];
  
  UISlider *rotSlider = [[UISlider alloc] init];
  rotSlider.minimumValue = -5;
  rotSlider.maximumValue = 5;
  rotSlider.value = 0;
  rotSlider.continuous = YES;
  [rotSlider addTarget:self
                     action:@selector(rotChanged:)
           forControlEvents:UIControlEventValueChanged];
  [rotSlider addTarget:self
                     action:@selector(rotReleased:)
           forControlEvents:UIControlEventTouchUpInside];
  [rotSlider addTarget:self
                     action:@selector(rotReleased:)
           forControlEvents:UIControlEventTouchUpOutside];
  rotSlider.translatesAutoresizingMaskIntoConstraints = NO;
  [self.scrollView addSubview:rotSlider];
  
  UILabel *zoomLabel = [[UILabel alloc] init];
  zoomLabel.backgroundColor = [UIColor clearColor];
  zoomLabel.textColor = [UIColor whiteColor];
  zoomLabel.textAlignment = NSTextAlignmentLeft;
  zoomLabel.text = @"Zoom";
  zoomLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.scrollView addSubview:zoomLabel];
  
  UISlider *zoomSlider = [[UISlider alloc] init];
  zoomSlider.minimumValue = -2;
  zoomSlider.maximumValue = 2;
  zoomSlider.value = 0;
  zoomSlider.continuous = YES;
  [zoomSlider addTarget:self
                     action:@selector(zoomChanged:)
           forControlEvents:UIControlEventValueChanged];
  [zoomSlider addTarget:self
                     action:@selector(zoomReleased:)
           forControlEvents:UIControlEventTouchUpInside];
  [zoomSlider addTarget:self
                     action:@selector(zoomReleased:)
           forControlEvents:UIControlEventTouchUpOutside];
  zoomSlider.translatesAutoresizingMaskIntoConstraints = NO;
  [self.scrollView addSubview:zoomSlider];
  
  UILabel *camDebugLabel = [[UILabel alloc] init];
  camDebugLabel.backgroundColor = [UIColor clearColor];
  camDebugLabel.textColor = [UIColor whiteColor];
  camDebugLabel.textAlignment = NSTextAlignmentCenter;
  camDebugLabel.text = @"Debug";
  camDebugLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.scrollView addSubview:camDebugLabel];
  
  self.camDebugSwitch = [[UISwitch alloc] init];
  self.camDebugSwitch.translatesAutoresizingMaskIntoConstraints = NO;
  self.camDebugSwitch.contentHorizontalAlignment =
      UIControlContentHorizontalAlignmentCenter;
  [self.camDebugSwitch addTarget:self
                     action:@selector(camDebugChanged:)
           forControlEvents:UIControlEventValueChanged];
  [self.scrollView addSubview:self.camDebugSwitch];
  
  UIButton *resetCamButton = [UIButton buttonWithType:UIButtonTypeRoundedRect];
  [resetCamButton setTitle:@"Reset" forState:UIControlStateNormal];
  resetCamButton.translatesAutoresizingMaskIntoConstraints = NO;
  [resetCamButton addTarget:self
                     action:@selector(handleCamResetDown:)
           forControlEvents:(UIControlEventTouchDown)];
  [resetCamButton addTarget:self
                     action:@selector(handleCamResetUp:)
           forControlEvents:(UIControlEventTouchUpInside)];
  [resetCamButton addTarget:self
                     action:@selector(handleCamResetUp:)
           forControlEvents:(UIControlEventTouchUpOutside)];
  [self.scrollView addSubview:resetCamButton];
  
  UIButton *restartSceneButton =
      [UIButton buttonWithType:UIButtonTypeRoundedRect];
  [restartSceneButton setTitle:@"Restart Scene" forState:UIControlStateNormal];
  restartSceneButton.translatesAutoresizingMaskIntoConstraints = NO;
  [restartSceneButton addTarget:self
                         action:@selector(restartScene:)
               forControlEvents:UIControlEventTouchUpInside];
  [self.scrollView addSubview:restartSceneButton];
  
  UIButton *rebootEngineButton =
      [UIButton buttonWithType:UIButtonTypeRoundedRect];
  [rebootEngineButton setTitle:@"Reboot Engine" forState:UIControlStateNormal];
  rebootEngineButton.translatesAutoresizingMaskIntoConstraints = NO;
  [rebootEngineButton addTarget:self
                         action:@selector(rebootEngine:)
               forControlEvents:UIControlEventTouchUpInside];
  [self.scrollView addSubview:rebootEngineButton];
  
  UILabel *gridLabel = [[UILabel alloc] init];
  gridLabel.backgroundColor = [UIColor clearColor];
  gridLabel.textColor = [UIColor whiteColor];
  gridLabel.textAlignment = NSTextAlignmentCenter;
  gridLabel.text = @"Draw World Grid";
  gridLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.scrollView addSubview:gridLabel];
  
  self.gridSwitch = [[UISwitch alloc] init];
  self.gridSwitch.translatesAutoresizingMaskIntoConstraints = NO;
  [self.gridSwitch addTarget:self
                 action:@selector(debugGridChanged:)
       forControlEvents:UIControlEventValueChanged];
  [self.scrollView addSubview:self.gridSwitch];
  
  UIButton *importSpriteButton =
      [UIButton buttonWithType:UIButtonTypeRoundedRect];
  [importSpriteButton setTitle:@"Import Sprite" forState:UIControlStateNormal];
  importSpriteButton.translatesAutoresizingMaskIntoConstraints = NO;
  [importSpriteButton addTarget:self
                         action:@selector(importSprite:)
               forControlEvents:UIControlEventTouchUpInside];
  [self.scrollView addSubview:importSpriteButton];
  
  UIButton *importTilesetButton =
      [UIButton buttonWithType:UIButtonTypeRoundedRect];
  [importTilesetButton setTitle:@"Import Tileset" forState:UIControlStateNormal];
  importTilesetButton.translatesAutoresizingMaskIntoConstraints = NO;
  [importTilesetButton addTarget:self
                         action:@selector(importTileset:)
               forControlEvents:UIControlEventTouchUpInside];
  [self.scrollView addSubview:importTilesetButton];
  
  /* LAYOUT CONSTRAINT BULLSHIT */
  NSDictionary *views =
      NSDictionaryOfVariableBindings(camLabel, envLabel, rotLabel,
                                     rotSlider, zoomLabel, zoomSlider,
                                     _scrollView, resetCamButton,
                                     camDebugLabel, _camDebugSwitch, _console,
                                     restartSceneButton, rebootEngineButton,
                                     gridLabel, _gridSwitch, resLabel,
                                     importSpriteButton, importTilesetButton);
  NSArray *headers =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[camLabel]-|"
          options:NSLayoutFormatAlignAllTop
          metrics:nil
          views:views];
  NSLayoutConstraint *header =
    [NSLayoutConstraint constraintWithItem:camLabel
                                 attribute:NSLayoutAttributeWidth
                                 relatedBy:NSLayoutRelationEqual
                                    toItem:self.scrollView
                                 attribute:NSLayoutAttributeWidth
                                multiplier:1.0
                                  constant:-40.0];
  [self.scrollView addConstraints:headers];
  [self.scrollView addConstraint:header];
  
  NSLayoutConstraint *consoleConstraint =
      [NSLayoutConstraint constraintWithItem:self.console
                                   attribute:NSLayoutAttributeWidth
                                   relatedBy:NSLayoutRelationEqual
                                      toItem:self.scrollView
                                   attribute:NSLayoutAttributeWidth
                                  multiplier:1.0
                                    constant:0];
  [self.scrollView addConstraint:consoleConstraint];
  
  consoleConstraint =
      [NSLayoutConstraint constraintWithItem:self.console
                                   attribute:NSLayoutAttributeLeft
                                   relatedBy:NSLayoutRelationEqual
                                      toItem:self.scrollView
                                   attribute:NSLayoutAttributeLeft
                                  multiplier:1.0
                                    constant:0];
  [self.scrollView addConstraint:consoleConstraint];
  
  NSArray *col =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"V:|[_console(120)]-(20)-[camLabel(20)]-[rotLabel(==camLabel)]-"
          @"[zoomLabel(==camLabel)]-(40)-[envLabel]"
          options:0
          metrics:nil
          views:views];
  [self.scrollView addConstraints:col];
  
  col =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"V:[envLabel]-[restartSceneButton(==resetCamButton)]-(40)-[resLabel]"
          @"-[importSpriteButton]"
          options:NSLayoutFormatAlignAllLeft
          metrics:nil
          views:views];
  [self.scrollView addConstraints:col];
  
  NSArray *row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[rotLabel(50)]-[rotSlider(<=camLabel)]-[camDebugLabel(==79)]-"
          @"[resetCamButton(60)]-|"
          options:NSLayoutFormatAlignAllTop
          metrics:nil
          views:views];
  [self.scrollView addConstraints:row];
  
  row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[zoomLabel(50)]-[zoomSlider(==rotSlider)]-"
          @"[_camDebugSwitch(==camDebugLabel)]-[resetCamButton(60)]-|"
          options:NSLayoutFormatAlignAllBottom
          metrics:nil
          views:views];
  [self.scrollView addConstraints:row];
  
  row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[restartSceneButton(180)]-(40)-[rebootEngineButton(180)]-(40)-[gridLabel]-[_gridSwitch(==79)]"
          options:NSLayoutFormatAlignAllTop
          metrics:nil
          views:views];
  [self.scrollView addConstraints:row];
  
  row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[importSpriteButton(180)]-[importTilesetButton(180)]"
          options:NSLayoutFormatAlignAllTop
          metrics:nil
          views:views];
  [self.scrollView addConstraints:row];
}

- (void)didReceiveMemoryWarning
{
  [super didReceiveMemoryWarning];
}

#pragma mark - Public

- (CGFloat)height {
  return kHeight;
}

- (int)pages {
  return kPages;
}

- (void)log:(NSString *)fmt arguments:(va_list)arguments {
  NSString *formatted =
      [[NSString alloc] initWithFormat:fmt arguments:arguments];
  [self.console logText:formatted];
}

#pragma mark - Debug controls

- (void)rotChanged:(UISlider *)sender {
  float newStep = roundf((sender.value) / 1.f);
  sender.value = newStep;
  _touchInput->cam_rotate = newStep;
}

- (void)rotReleased:(UISlider *)sender {
  sender.value = 0;
  _touchInput->cam_rotate = 0;
}

- (void)zoomChanged:(UISlider *)sender {
  float newStep = roundf((sender.value) / 1.f);
  sender.value = newStep;
  _touchInput->cam_zoom = newStep;
}

- (void)zoomReleased:(UISlider *)sender {
  sender.value = 0;
  _touchInput->cam_zoom = 0;
}

- (void)handleCamResetDown:(UIButton *)sender {
  _touchInput->cam_reset = 1;
}

- (void)handleCamResetUp:(UIButton *)sender {
  _touchInput->cam_reset = 0;
}

- (void)restartScene:(UIButton *)sender {
  [self.engineVC restartScene];
}

- (void)rebootEngine:(UIButton *)sender {
  [self.engineVC reboot];
}

- (void)importGraphicalResourceOfKind:(GraphicalResourceKind)resourceKind
                               sender:(UIView *)sender {
  UIImagePickerController *picker = [[UIImagePickerController alloc] init];
  picker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
  picker.delegate = self;
  self.importKind = resourceKind;
  
  if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
    UIPopoverController *popover =
        [[UIPopoverController alloc] initWithContentViewController:picker];
    [popover presentPopoverFromRect:sender.frame
                             inView:self.view
           permittedArrowDirections:UIPopoverArrowDirectionAny
                           animated:YES];
    self.popover = popover;
  } else {
      [self presentViewController:picker
                         animated:YES
                       completion:^{
                         
                       }];
  }
}

- (void)importSprite:(UIButton *)sender {
  [self importGraphicalResourceOfKind:GraphicalResourceKindSprite
                               sender:sender];
}

- (void)importTileset:(UIButton *)sender {
  [self importGraphicalResourceOfKind:GraphicalResourceKindTileset
                               sender:sender];
}

- (void)imagePickerController:(UIImagePickerController *)picker
didFinishPickingMediaWithInfo:(NSDictionary *)info {
  self.importImage = [info objectForKey:UIImagePickerControllerOriginalImage];
  
  self.importAlert = [[UIAlertView alloc] initWithTitle:@"Name this file"
                             message:@".png will be appended"
                            delegate:self
                   cancelButtonTitle:@"Cancel"
                   otherButtonTitles:@"Save", nil];
  self.importAlert.alertViewStyle = UIAlertViewStylePlainTextInput;
  [self.importAlert show];
}

- (void)alertView:(UIAlertView *)alertView
    didDismissWithButtonIndex:(NSInteger)buttonIndex {
  if (alertView == self.importAlert) {
    if (buttonIndex == 1) {
      NSArray *bundlePath =
          NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                              NSUserDomainMask, YES);
      NSString *subdir;
      switch (self.importKind) {
        case GraphicalResourceKindSprite:
          subdir = @"media/sprites/";
          break;
          
        case GraphicalResourceKindTileset:
          subdir = @"media/tilesets/";
          break;
      }
      
      NSString *filename = [alertView textFieldAtIndex:0].text;
      NSString *dir = [[bundlePath objectAtIndex:0]
                           stringByAppendingPathComponent:subdir];
      NSString *resourceName =
          [[dir stringByAppendingPathComponent:filename]
              stringByAppendingPathExtension:@"png"];
      NSFileManager *manager = [NSFileManager defaultManager];
      NSError *error = nil;
      [manager createDirectoryAtPath:dir withIntermediateDirectories:YES
                          attributes:nil
                               error:&error];
      if (error) {
        NSLog(@"%@", error);
      } else {
        if ([manager isWritableFileAtPath:dir]) {
          [UIImagePNGRepresentation(self.importImage) writeToFile:resourceName
                                                       atomically:NO];
          Engine_log_iOS("Imported <%@>",
                         [[subdir stringByAppendingPathComponent:filename]
                          stringByAppendingPathExtension:@"png"]);
        } else {
          Engine_log_iOS("Failed to import <%@>",
                         [[subdir stringByAppendingPathComponent:filename]
                          stringByAppendingPathExtension:@"png"]);
        }
        
      }
      [self.popover dismissPopoverAnimated:YES];
      self.popover = nil;
    }
    
    self.importAlert = nil;
    self.importImage = nil;
  }
}

- (void)camDebugChanged:(UISwitch *)sender {
  self.engineVC.scene->debug_camera = sender.on;
}

- (void)debugGridChanged:(UISwitch *)sender {
  self.engineVC.scene->draw_grid = sender.on;
}

#pragma mark - Scroll View Delegate

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
  [self.engineVC performSelector:@selector(fullUpdate)];
}

@end
