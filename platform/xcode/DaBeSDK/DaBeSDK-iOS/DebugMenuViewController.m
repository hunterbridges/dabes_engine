//
//  DebugMenuViewController.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/4/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <GameKit/GameKit.h>
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
@property (nonatomic, strong) IBOutlet UIButton *recorderLinkButton;
@property (nonatomic, strong) IBOutlet UIButton *recorderRecordButton;
@property (nonatomic, strong) IBOutlet UIButton *recorderPlayButton;
@property (nonatomic, strong) IBOutlet UIButton *recorderSendButton;
@property (nonatomic, strong) IBOutlet UIButton *recorderConnectButton;
@property (nonatomic, assign) Recorder *debugRecorder;

@property (nonatomic, assign) BOOL recorderEditingLink;

@property (nonatomic, assign) GraphicalResourceKind importKind;
@property (nonatomic, strong) UIImage *importImage;
@property (nonatomic, strong) UIAlertView *importAlert;

@property (nonatomic, strong) GKMatch *match;

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

- (void)viewDidLoad {
  [super viewDidLoad];
  [[NSNotificationCenter defaultCenter]
      addObserver:self
      selector:@selector(handleFrameEndNotification:)
      name:kFrameEndNotification
      object:nil];
  [[NSNotificationCenter defaultCenter]
      addObserver:self
      selector:@selector(handleEntitySelectedNotification:)
      name:kEntitySelectedNotification
      object:nil];
}

- (void)dealloc {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)viewDidAppear:(BOOL)animated {
  [super viewDidAppear:animated];
  [self.scrollView flashScrollIndicators];
}

#pragma mark - Notification Handlers

- (void)handleFrameEndNotification:(NSNotification *)notif {
  if (self.debugRecorder &&
      self.debugRecorder->state != RecorderStatePlaying &&
      self.recorderPlayButton.selected) {
    [self stopPlayback];
  }
}

- (void)handleEntitySelectedNotification:(NSNotification *)notif {
  if (self.engineVC.scene->selection_mode == kSceneSelectingForRecorder) {
    Entity *entity = List_pop(self.engineVC.scene->selected_entities);
    if (self.debugRecorder) {
      self.debugRecorder->entity = entity;
      self.debugRecorder->_(rewind)(self.debugRecorder);
      self.debugRecorder->_(clear_frames)(self.debugRecorder);
    } else {
      self.debugRecorder = self.engineVC.scene->_(gen_recorder)(self.engineVC.scene, entity);
    }
    
    Scene_set_selection_mode(self.engineVC.scene, kSceneNotSelecting);
    self.recorderEditingLink = NO;
    self.recorderLinkButton.selected = NO;
    self.recorderRecordButton.enabled = YES;
    self.recorderPlayButton.enabled = YES;
  }
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

- (IBAction)recorderLinkEntityClicked:(UIButton *)sender {
  if (sender.selected) {
    sender.selected = NO;
    self.recorderEditingLink = NO;
    Scene_set_selection_mode(self.engineVC.scene, kSceneNotSelecting);
  } else {
    sender.selected = YES;
    Scene_set_selection_mode(self.engineVC.scene, kSceneSelectingForRecorder);
    self.recorderRecordButton.enabled = NO;
    self.recorderPlayButton.enabled = NO;
  }
}

- (void)stopPlayback {
  Recorder_set_state(self.debugRecorder, RecorderStateIdle);
  self.recorderPlayButton.selected = NO;
  [self.recorderRecordButton setEnabled:YES];
}

- (void)startPlayback {
  Recorder_set_state(self.debugRecorder, RecorderStatePlaying);
  self.recorderPlayButton.selected = YES;
  [self.recorderRecordButton setEnabled:NO];
}

- (IBAction)recorderRecordClicked:(id)sender {
  UIButton *button = sender;
  if (button.selected) {
    Recorder_set_state(self.debugRecorder, RecorderStateIdle);
    button.selected = NO;
    [self.recorderPlayButton setEnabled:YES];
  } else {
    Recorder_set_state(self.debugRecorder, RecorderStateRecording);
    button.selected = YES;
    [self.recorderPlayButton setEnabled:NO];
  }
}

- (IBAction)recorderPlayClicked:(id)sender {
  UIButton *button = sender;
  if (button.selected) {
    [self stopPlayback];
  } else {
    [self startPlayback];
  }
}

- (IBAction)recorderSendClicked:(id)sender {
  unsigned char *packed = NULL;
  size_t sz = 0;
  self.debugRecorder->_(pack)(self.debugRecorder, &packed, &sz);
  
  NSData *data = [[NSData alloc] initWithBytes:packed length:sz];
  NSError *error = nil;
  [self.match sendDataToAllPlayers:data withDataMode:GKMatchSendDataReliable error:&error];
  if (error) {
    
  }
  
  free(packed);
}

- (IBAction)recorderConnectClicked:(id)sender {
  __weak __typeof(self) wSelf = self;
  [GKLocalPlayer localPlayer].authenticateHandler =
    ^(UIViewController *viewController, NSError *error) {
      if (viewController) {
        [wSelf presentViewController:viewController animated:YES completion:nil];
      } else if ([GKLocalPlayer localPlayer].isAuthenticated) {
        GKMatchRequest *request = [[GKMatchRequest alloc] init];
        request.minPlayers = 2;
        request.maxPlayers = 2;
        request.defaultNumberOfPlayers = 2;
        
        GKMatchmakerViewController *mmvc = [[GKMatchmakerViewController alloc] initWithMatchRequest:request];
        mmvc.matchmakerDelegate = wSelf;
        
        [wSelf presentViewController:mmvc animated:YES completion:nil];
      }
    };
}

#pragma mark - Matchmaker View Controller Delegate

- (void)matchmakerViewControllerWasCancelled:(GKMatchmakerViewController *)viewController {
  [self dismissViewControllerAnimated:YES completion:nil];
}

- (void)matchmakerViewController:(GKMatchmakerViewController *)viewController didFindMatch:(GKMatch *)match {
  [self dismissViewControllerAnimated:YES completion:nil];
  self.match = match;
  match.delegate = self;
  [match chooseBestHostPlayerWithCompletionHandler:^(NSString *playerID) {
    
  }];
}

- (void)matchmakerViewController:(GKMatchmakerViewController *)viewController
                didFailWithError:(NSError *)error {
  [self dismissViewControllerAnimated:YES completion:nil];
}

#pragma mark - Match Delegate

- (void)match:(GKMatch *)match didReceiveData:(NSData *)data fromPlayer:(NSString *)playerID {
  if (self.debugRecorder) {
    Recorder_set_state(self.debugRecorder, RecorderStateRecording);
    self.debugRecorder->_(unpack)(self.debugRecorder,
                                  (unsigned char *)data.bytes,
                                  data.length);
    Recorder_set_state(self.debugRecorder, RecorderStateIdle);
    [self startPlayback];
  }
}

#pragma mark - Scroll View Delegate

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
  [self.engineVC performSelector:@selector(fullUpdate)];
}

@end
