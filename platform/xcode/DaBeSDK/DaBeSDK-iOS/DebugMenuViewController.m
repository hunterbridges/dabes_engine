//
//  DebugMenuViewController.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/4/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import "DebugMenuViewController.h"

#import "ConsoleView.h"
#import "SDKiOSProjectEngineViewController.h"

static const CGFloat kHeight = 120.f;
static const int kPages = 5;

@interface DebugMenuViewController ()
  <UIScrollViewDelegate, UIImagePickerControllerDelegate,
      UINavigationControllerDelegate, UIAlertViewDelegate> {
}

@property (nonatomic, strong) IBOutlet UIScrollView *scrollView;

@property (nonatomic, strong) UIView *contentView;
@property (nonatomic, strong) IBOutlet ConsoleView *console;
@property (nonatomic, strong) UIPopoverController *popover;
@property (nonatomic, strong) IBOutlet UISlider *camRotSlider;
@property (nonatomic, strong) IBOutlet UISlider *camZoomSlider;
@property (nonatomic, strong) IBOutlet UISwitch *camDebugSwitch;
@property (nonatomic, strong) IBOutlet UIButton *camResetButton;
@property (nonatomic, strong) IBOutlet UISwitch *physRenderSwitch;
@property (nonatomic, strong) IBOutlet UISwitch *gridSwitch;
@property (nonatomic, strong) IBOutlet UIButton *restartSceneButton;
@property (nonatomic, strong) IBOutlet UIButton *rebootEngineButton;
@property (nonatomic, strong) IBOutlet UIButton *importSpriteButton;
@property (nonatomic, strong) IBOutlet UIButton *importTilesetButton;

@property (nonatomic, assign) GraphicalResourceKind importKind;
@property (nonatomic, strong) UIImage *importImage;
@property (nonatomic, strong) UIAlertView *importAlert;
@end

@implementation DebugMenuViewController

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

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];
  [self.scrollView flashScrollIndicators];
}

#pragma mark - Debug controls

- (IBAction)rotChanged:(UISlider *)sender {
  float newStep = roundf((sender.value) / 1.f);
  sender.value = newStep;
  _touchInput->cam_rotate = newStep;
}

- (IBAction)rotReleased:(UISlider *)sender {
  sender.value = 0;
  _touchInput->cam_rotate = 0;
}

- (IBAction)zoomChanged:(UISlider *)sender {
  float newStep = roundf((sender.value) / 1.f);
  sender.value = newStep;
  _touchInput->cam_zoom = newStep;
}

- (IBAction)zoomReleased:(UISlider *)sender {
  sender.value = 0;
  _touchInput->cam_zoom = 0;
}

- (IBAction)handleCamResetDown:(UIButton *)sender {
  _touchInput->cam_reset = 1;
}

- (IBAction)handleCamResetUp:(UIButton *)sender {
  _touchInput->cam_reset = 0;
}

- (IBAction)restartScene:(UIButton *)sender {
  [self.engineVC restartScene];
}

- (IBAction)rebootEngine:(UIButton *)sender {
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

- (IBAction)importSprite:(UIButton *)sender {
  [self importGraphicalResourceOfKind:GraphicalResourceKindSprite
                               sender:sender];
}

- (IBAction)importTileset:(UIButton *)sender {
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

- (IBAction)camDebugChanged:(UISwitch *)sender {
  self.engineVC.scene->debug_camera = sender.on;
}

- (IBAction)physRenderChanged:(UISwitch *)sender {
  self.engineVC.scene->render_mode = sender.on;
}

- (IBAction)debugGridChanged:(UISwitch *)sender {
  self.engineVC.scene->draw_grid = sender.on;
}

#pragma mark - Scroll View Delegate

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
  [self.engineVC performSelector:@selector(fullUpdate)];
}

@end
