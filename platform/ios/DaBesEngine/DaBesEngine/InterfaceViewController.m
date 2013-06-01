#import <AssetsLibrary/AssetsLibrary.h>
#import <QuartzCore/QuartzCore.h>
#import "ConsoleView.h"
#import "InterfaceViewController.h"
#import "EngineViewController.h"
#import "ScriptEditorViewController.h"
#import "CodeEditorView.h"

@interface InterfaceViewController () <UIGestureRecognizerDelegate> {
  EngineViewController *engineVC_;
  BOOL showingMenu_;
  BOOL showingScriptEditor_;
  
  UISwipeGestureRecognizer *menuSwipe_;
  UIScrollView *debugMenuView_;
  float menuHeight_;
  int pages_;
  ConsoleView *console_;
  ScriptEditorViewController *scriptEditor_;
  UIPopoverController *popover_;
  UISwitch *camDebugSwitch_;
  UISwitch *gridSwitch_;
  
  GraphicalResourceKind importKind_;
  UIImage *importImage_;
  UIAlertView *importAlert_;
}

@end

@implementation InterfaceViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  if (touchInput_ == NULL) {
    touchInput_ = Input_create();
  }
  
  self.view.backgroundColor = [UIColor colorWithWhite:0.65 alpha:1];
  
  [self buildDebugMenu];
  
  engineVC_ = [[EngineViewController alloc] initWithTouchInput:touchInput_];
  [self addChildViewController:engineVC_];
  [self.view addSubview:engineVC_.view];
  
  scriptEditor_ =
      [[ScriptEditorViewController alloc] initWithEngine:engineVC_.engine];
  
  UISwipeGestureRecognizer *downSwipe =
      [[UISwipeGestureRecognizer alloc] initWithTarget:self
                                                action:@selector(handleSwipe:)];
  downSwipe.numberOfTouchesRequired = 3;
  downSwipe.direction = UISwipeGestureRecognizerDirectionDown;
  [self.view addGestureRecognizer:downSwipe];
  
  UISwipeGestureRecognizer *upSwipe =
      [[UISwipeGestureRecognizer alloc] initWithTarget:self
                                                action:@selector(handleSwipe:)];
  upSwipe.numberOfTouchesRequired = 3;
  upSwipe.direction = UISwipeGestureRecognizerDirectionUp;
  [self.view addGestureRecognizer:upSwipe];
  
  UISwipeGestureRecognizer *leftSwipe =
      [[UISwipeGestureRecognizer alloc] initWithTarget:self
                                                action:@selector(handleSwipe:)];
  leftSwipe.numberOfTouchesRequired = 3;
  leftSwipe.direction = UISwipeGestureRecognizerDirectionLeft;
  [self.view addGestureRecognizer:leftSwipe];
  
  UISwipeGestureRecognizer *rightSwipe =
      [[UISwipeGestureRecognizer alloc] initWithTarget:self
                                                action:@selector(handleSwipe:)];
  rightSwipe.numberOfTouchesRequired = 3;
  rightSwipe.direction = UISwipeGestureRecognizerDirectionRight;
  [self.view addGestureRecognizer:rightSwipe];
  
}

- (void)viewWillAppear:(BOOL)animated {
  [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillShow:) name:UIKeyboardWillShowNotification object:nil];
  [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(keyboardWillHide:) name:UIKeyboardWillHideNotification object:nil];
}

- (void)viewDidAppear:(BOOL)animated {
  [self showScriptEditor];
}

- (void)viewWillDisappear:(BOOL)animated {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)dealloc {
  Input_destroy(touchInput_);
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
}

- (void)viewDidLayoutSubviews {
  debugMenuView_.contentSize = CGSizeMake(self.view.bounds.size.width,
                                         menuHeight_ * pages_);
  if (showingScriptEditor_) {
     scriptEditor_.view.frame = CGRectMake(self.view.bounds.size.width / 2,
                                           0,
                                           self.view.bounds.size.width / 2,
                                           self.view.bounds.size.height);
     engineVC_.view.frame = CGRectMake(0, 0,
                                       self.view.bounds.size.width / 2,
                                       self.view.bounds.size.height);
  }
}

#pragma mark - Public

- (void)didEnterBackground {
  if ([engineVC_.view superview]) [engineVC_ didEnterBackground];
}

- (void)willEnterForeground {
  if ([engineVC_.view superview]) [engineVC_ willEnterForeground];
}

- (void)injectMapFromPath:(NSString *)newFilePath {
  [engineVC_ injectMapFromPath:newFilePath];
}

- (void)log:(NSString *)fmt arguments:(va_list)arguments {
  NSString *formatted =
      [[NSString alloc] initWithFormat:fmt arguments:arguments];
  [console_ logText:formatted];
  [self showMenu];
}

#pragma mark - Private

- (void)showMenu {
  if (showingMenu_) return;
  if (showingScriptEditor_) return;
  [UIView animateWithDuration:0.5
       animations:^{
         engineVC_.view.frame = CGRectMake(0, menuHeight_,
                                           self.view.bounds.size.width,
                                           self.view.bounds.size.height
                                               - menuHeight_);
       }];
  showingMenu_ = YES;
}

- (void)hideMenu {
  if (!showingMenu_) return;
  [UIView animateWithDuration:0.5
       animations:^{
         engineVC_.view.frame = CGRectMake(0, 0,
                                           self.view.bounds.size.width,
                                           self.view.bounds.size.height);
       }];
  showingMenu_ = NO;
}

- (void)showScriptEditor {
  if (showingMenu_) return;
  if (showingScriptEditor_) return;
  showingScriptEditor_ = YES;
  scriptEditor_.view.frame = CGRectMake(self.view.bounds.size.width,
                                        0,
                                        self.view.bounds.size.width / 2,
                                        self.view.bounds.size.height);
  scriptEditor_.editorView.linkTo = engineVC_;
  [self addChildViewController:scriptEditor_];
  [self.view addSubview:scriptEditor_.view];
  [UIView animateWithDuration:0.5
       animations:^{
         scriptEditor_.view.frame = CGRectMake(self.view.bounds.size.width / 2,
                                               0,
                                               self.view.bounds.size.width / 2,
                                               self.view.bounds.size.height);
         engineVC_.view.frame = CGRectMake(0, 0,
                                           self.view.bounds.size.width / 2,
                                           self.view.bounds.size.height);
       }];
  
}

- (void)hideScriptEditor {
  if (!showingScriptEditor_) return;

  [UIView animateWithDuration:0.5
       animations:^{
         scriptEditor_.view.frame = CGRectMake(self.view.bounds.size.width,
                                               0,
                                               self.view.bounds.size.width / 2,
                                               self.view.bounds.size.height);
         engineVC_.view.frame = CGRectMake(0, 0,
                                           self.view.bounds.size.width,
                                           self.view.bounds.size.height);
       }
       completion:^(BOOL finished) {
         [scriptEditor_ removeFromParentViewController];
         [scriptEditor_.view removeFromSuperview];
         showingScriptEditor_ = NO;
       }];
}

- (void)buildDebugMenu {
  UIImageView *blueprint =
      [[UIImageView alloc] initWithImage:[UIImage imageNamed:@"blueprint.jpg"]];
  blueprint.frame = self.view.bounds;
  blueprint.autoresizingMask = (UIViewAutoresizingFlexibleHeight |
                                UIViewAutoresizingFlexibleWidth);
  blueprint.contentMode = UIViewContentModeScaleAspectFill;
  [self.view addSubview:blueprint];
  
  menuHeight_ = 120.f;
  pages_ = 5;
  CGRect debugMenuFrame = CGRectMake(0, 0, self.view.bounds.size.width,
                                     menuHeight_);
  debugMenuView_ =
      [[UIScrollView alloc] initWithFrame:debugMenuFrame];
  debugMenuView_.autoresizingMask = UIViewAutoresizingFlexibleWidth;
  debugMenuView_.pagingEnabled = YES;
  debugMenuView_.scrollEnabled = YES;
  debugMenuView_.translatesAutoresizingMaskIntoConstraints = YES;
  debugMenuView_.userInteractionEnabled = YES;
  debugMenuView_.delegate = self;
  debugMenuView_.delaysContentTouches = YES;
  debugMenuView_.clipsToBounds = NO;
  debugMenuView_.delegate = self;
  [self.view addSubview:debugMenuView_];
  
  console_ = [[ConsoleView alloc] init];
  console_.translatesAutoresizingMaskIntoConstraints = NO;
  [debugMenuView_ addSubview:console_];
  
  UILabel *camLabel = [[UILabel alloc] init];
  camLabel.backgroundColor = [UIColor clearColor];
  camLabel.textColor = [UIColor whiteColor];
  camLabel.textAlignment = NSTextAlignmentLeft;
  camLabel.text = @"Camera";
  camLabel.font = [UIFont boldSystemFontOfSize:20];
  camLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [debugMenuView_ addSubview:camLabel];
  
  UILabel *envLabel = [[UILabel alloc] init];
  envLabel.backgroundColor = [UIColor clearColor];
  envLabel.textColor = [UIColor whiteColor];
  envLabel.textAlignment = NSTextAlignmentLeft;
  envLabel.text = @"Environment";
  envLabel.font = [UIFont boldSystemFontOfSize:20];
  envLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [debugMenuView_ addSubview:envLabel];
  
  UILabel *resLabel = [[UILabel alloc] init];
  resLabel.backgroundColor = [UIColor clearColor];
  resLabel.textColor = [UIColor whiteColor];
  resLabel.textAlignment = NSTextAlignmentLeft;
  resLabel.text = @"Resources";
  resLabel.font = [UIFont boldSystemFontOfSize:20];
  resLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [debugMenuView_ addSubview:resLabel];
  
  UILabel *rotLabel = [[UILabel alloc] init];
  rotLabel.backgroundColor = [UIColor clearColor];
  rotLabel.textColor = [UIColor whiteColor];
  rotLabel.textAlignment = NSTextAlignmentLeft;
  rotLabel.text = @"Rot";
  rotLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [debugMenuView_ addSubview:rotLabel];
  
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
  [debugMenuView_ addSubview:rotSlider];
  
  UILabel *zoomLabel = [[UILabel alloc] init];
  zoomLabel.backgroundColor = [UIColor clearColor];
  zoomLabel.textColor = [UIColor whiteColor];
  zoomLabel.textAlignment = NSTextAlignmentLeft;
  zoomLabel.text = @"Zoom";
  zoomLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [debugMenuView_ addSubview:zoomLabel];
  
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
  [debugMenuView_ addSubview:zoomSlider];
  
  UILabel *camDebugLabel = [[UILabel alloc] init];
  camDebugLabel.backgroundColor = [UIColor clearColor];
  camDebugLabel.textColor = [UIColor whiteColor];
  camDebugLabel.textAlignment = NSTextAlignmentCenter;
  camDebugLabel.text = @"Debug";
  camDebugLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [debugMenuView_ addSubview:camDebugLabel];
  
  camDebugSwitch_ = [[UISwitch alloc] init];
  camDebugSwitch_.translatesAutoresizingMaskIntoConstraints = NO;
  camDebugSwitch_.contentHorizontalAlignment =
      UIControlContentHorizontalAlignmentCenter;
  [camDebugSwitch_ addTarget:self
                     action:@selector(camDebugChanged:)
           forControlEvents:UIControlEventValueChanged];
  [debugMenuView_ addSubview:camDebugSwitch_];
  
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
  [debugMenuView_ addSubview:resetCamButton];
  
  UIButton *restartSceneButton =
      [UIButton buttonWithType:UIButtonTypeRoundedRect];
  [restartSceneButton setTitle:@"Restart Scene" forState:UIControlStateNormal];
  restartSceneButton.translatesAutoresizingMaskIntoConstraints = NO;
  [restartSceneButton addTarget:self
                         action:@selector(restartScene:)
               forControlEvents:UIControlEventTouchUpInside];
  [debugMenuView_ addSubview:restartSceneButton];
  
  UIButton *rebootEngineButton =
      [UIButton buttonWithType:UIButtonTypeRoundedRect];
  [rebootEngineButton setTitle:@"Reboot Engine" forState:UIControlStateNormal];
  rebootEngineButton.translatesAutoresizingMaskIntoConstraints = NO;
  [rebootEngineButton addTarget:self
                         action:@selector(rebootEngine:)
               forControlEvents:UIControlEventTouchUpInside];
  [debugMenuView_ addSubview:rebootEngineButton];
  
  UILabel *gridLabel = [[UILabel alloc] init];
  gridLabel.backgroundColor = [UIColor clearColor];
  gridLabel.textColor = [UIColor whiteColor];
  gridLabel.textAlignment = NSTextAlignmentCenter;
  gridLabel.text = @"Draw World Grid";
  gridLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [debugMenuView_ addSubview:gridLabel];
  
  gridSwitch_ = [[UISwitch alloc] init];
  gridSwitch_.translatesAutoresizingMaskIntoConstraints = NO;
  [gridSwitch_ addTarget:self
                 action:@selector(debugGridChanged:)
       forControlEvents:UIControlEventValueChanged];
  [debugMenuView_ addSubview:gridSwitch_];
  
  UIButton *importSpriteButton =
      [UIButton buttonWithType:UIButtonTypeRoundedRect];
  [importSpriteButton setTitle:@"Import Sprite" forState:UIControlStateNormal];
  importSpriteButton.translatesAutoresizingMaskIntoConstraints = NO;
  [importSpriteButton addTarget:self
                         action:@selector(importSprite:)
               forControlEvents:UIControlEventTouchUpInside];
  [debugMenuView_ addSubview:importSpriteButton];
  
  UIButton *importTilesetButton =
      [UIButton buttonWithType:UIButtonTypeRoundedRect];
  [importTilesetButton setTitle:@"Import Tileset" forState:UIControlStateNormal];
  importTilesetButton.translatesAutoresizingMaskIntoConstraints = NO;
  [importTilesetButton addTarget:self
                         action:@selector(importTileset:)
               forControlEvents:UIControlEventTouchUpInside];
  [debugMenuView_ addSubview:importTilesetButton];
  
  /* LAYOUT CONSTRAINT BULLSHIT */
  NSDictionary *views =
      NSDictionaryOfVariableBindings(camLabel, envLabel, rotLabel,
                                     rotSlider, zoomLabel, zoomSlider,
                                     debugMenuView_, resetCamButton,
                                     camDebugLabel, camDebugSwitch_, console_,
                                     restartSceneButton, rebootEngineButton,
                                     gridLabel, gridSwitch_, resLabel,
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
                                    toItem:debugMenuView_
                                 attribute:NSLayoutAttributeWidth
                                multiplier:1.0
                                  constant:-40.0];
  [debugMenuView_ addConstraints:headers];
  [debugMenuView_ addConstraint:header];
  
  NSLayoutConstraint *consoleConstraint =
      [NSLayoutConstraint constraintWithItem:console_
                                   attribute:NSLayoutAttributeWidth
                                   relatedBy:NSLayoutRelationEqual
                                      toItem:debugMenuView_
                                   attribute:NSLayoutAttributeWidth
                                  multiplier:1.0
                                    constant:0];
  [debugMenuView_ addConstraint:consoleConstraint];
  
  consoleConstraint =
      [NSLayoutConstraint constraintWithItem:console_
                                   attribute:NSLayoutAttributeLeft
                                   relatedBy:NSLayoutRelationEqual
                                      toItem:debugMenuView_
                                   attribute:NSLayoutAttributeLeft
                                  multiplier:1.0
                                    constant:0];
  [debugMenuView_ addConstraint:consoleConstraint];
  
  NSArray *col =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"V:|[console_(120)]-(20)-[camLabel(20)]-[rotLabel(==camLabel)]-"
          @"[zoomLabel(==camLabel)]-(40)-[envLabel]"
          options:0
          metrics:nil
          views:views];
  [debugMenuView_ addConstraints:col];
  
  col =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"V:[envLabel]-[restartSceneButton(==resetCamButton)]-(40)-[resLabel]"
          @"-[importSpriteButton]"
          options:NSLayoutFormatAlignAllLeft
          metrics:nil
          views:views];
  [debugMenuView_ addConstraints:col];
  
  NSArray *row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[rotLabel(50)]-[rotSlider(<=camLabel)]-[camDebugLabel(==79)]-"
          @"[resetCamButton(60)]-|"
          options:NSLayoutFormatAlignAllTop
          metrics:nil
          views:views];
  [debugMenuView_ addConstraints:row];
  
  row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[zoomLabel(50)]-[zoomSlider(==rotSlider)]-"
          @"[camDebugSwitch_(==camDebugLabel)]-[resetCamButton(60)]-|"
          options:NSLayoutFormatAlignAllBottom
          metrics:nil
          views:views];
  [debugMenuView_ addConstraints:row];
  
  row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[restartSceneButton(180)]-(40)-[rebootEngineButton(180)]-(40)-[gridLabel]-[gridSwitch_(==79)]"
          options:NSLayoutFormatAlignAllTop
          metrics:nil
          views:views];
  [debugMenuView_ addConstraints:row];
  
  row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[importSpriteButton(180)]-[importTilesetButton(180)]"
          options:NSLayoutFormatAlignAllTop
          metrics:nil
          views:views];
  [debugMenuView_ addConstraints:row];
}

#pragma mark - Debug controls
- (void)rotChanged:(UISlider *)sender {
  float newStep = roundf((sender.value) / 1.f);
  sender.value = newStep;
  touchInput_->cam_rotate = newStep;
}

- (void)rotReleased:(UISlider *)sender {
  sender.value = 0;
  touchInput_->cam_rotate = 0;
}

- (void)zoomChanged:(UISlider *)sender {
  float newStep = roundf((sender.value) / 1.f);
  sender.value = newStep;
  touchInput_->cam_zoom = newStep;
}

- (void)zoomReleased:(UISlider *)sender {
  sender.value = 0;
  touchInput_->cam_zoom = 0;
}

- (void)handleCamResetDown:(UIButton *)sender {
  touchInput_->cam_reset = 1;
}

- (void)handleCamResetUp:(UIButton *)sender {
  touchInput_->cam_reset = 0;
}

- (void)restartScene:(UIButton *)sender {
  [engineVC_ restartScene];
}

- (void)rebootEngine:(UIButton *)sender {
  [engineVC_ reboot];
}

- (void)importGraphicalResourceOfKind:(GraphicalResourceKind)resourceKind
                               sender:(UIView *)sender {
  UIImagePickerController *picker = [[UIImagePickerController alloc] init];
  picker.sourceType = UIImagePickerControllerSourceTypePhotoLibrary;
  picker.delegate = self;
  importKind_ = resourceKind;
  
  if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPad) {
    UIPopoverController *popover =
        [[UIPopoverController alloc] initWithContentViewController:picker];
    [popover presentPopoverFromRect:sender.frame
                             inView:debugMenuView_
           permittedArrowDirections:UIPopoverArrowDirectionAny
                           animated:YES];
    popover_ = popover;
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
  importImage_ = [info objectForKey:UIImagePickerControllerOriginalImage];
  
  importAlert_ = [[UIAlertView alloc] initWithTitle:@"Name this file"
                             message:@".png will be appended"
                            delegate:self
                   cancelButtonTitle:@"Cancel"
                   otherButtonTitles:@"Save", nil];
  importAlert_.alertViewStyle = UIAlertViewStylePlainTextInput;
  [importAlert_ show];
}

- (void)alertView:(UIAlertView *)alertView
    didDismissWithButtonIndex:(NSInteger)buttonIndex {
  if (alertView == importAlert_) {
    if (buttonIndex == 1) {
      NSArray *bundlePath =
          NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                              NSUserDomainMask, YES);
      NSString *subdir;
      switch (importKind_) {
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
          [UIImagePNGRepresentation(importImage_) writeToFile:resourceName
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
      [popover_ dismissPopoverAnimated:YES];
      popover_ = nil;
    }
    
    importAlert_ = nil;
    importImage_ = nil;
  }
}

- (void)camDebugChanged:(UISwitch *)sender {
  engineVC_.scene->debug_camera = sender.on;
}

- (void)debugGridChanged:(UISwitch *)sender {
  engineVC_.scene->draw_grid = sender.on;
}

#pragma mark - Scroll View Delegate

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
  GLKView *view = (GLKView *)engineVC_.view;
  [engineVC_ performSelector:@selector(update)];
  [engineVC_ glkView:view drawInRect:view.bounds];
  [view display];
}


#pragma mark - Gesture Recognizer

- (void)handleSwipe:(UISwipeGestureRecognizer *)gesture {
  if (gesture.direction == UISwipeGestureRecognizerDirectionDown) {
    [self showMenu];
  } else if (gesture.direction == UISwipeGestureRecognizerDirectionUp) {
    [self hideMenu];
  }
  
  if (gesture.direction == UISwipeGestureRecognizerDirectionLeft) {
    [self showScriptEditor];
  } else if (gesture.direction == UISwipeGestureRecognizerDirectionRight) {
    [self hideScriptEditor];
  }
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer shouldRecognizeSimultaneouslyWithGestureRecognizer:(UIGestureRecognizer *)otherGestureRecognizer {
  return YES;
}

#pragma mark - Keyboard Handling

- (void)keyboardWillShow:(NSNotification *)notif {
  [self moveViewsForKeyboard:notif up:YES];
}

- (void)keyboardWillHide:(NSNotification *)notif {
  [self moveViewsForKeyboard:notif up:NO];
}

- (void)moveViewsForKeyboard:(NSNotification*)notification up:(BOOL)up {
  NSDictionary* userInfo = [notification userInfo];
  NSTimeInterval animationDuration;
  UIViewAnimationCurve animationCurve;
  CGRect keyboardEndFrame;
  
  [[userInfo objectForKey:UIKeyboardAnimationCurveUserInfoKey] getValue:&animationCurve];
  [[userInfo objectForKey:UIKeyboardAnimationDurationUserInfoKey] getValue:&animationDuration];
  [[userInfo objectForKey:UIKeyboardFrameEndUserInfoKey] getValue:&keyboardEndFrame];
  
  [UIView beginAnimations:nil context:nil];
  [UIView setAnimationDuration:animationDuration];
  [UIView setAnimationCurve:animationCurve];
  
  CGRect newFrame = self.view.bounds;
  CGRect keyboardFrame = [self.view convertRect:keyboardEndFrame toView:nil];
  if (up) newFrame.size.height -= keyboardFrame.size.height;
  engineVC_.view.frame = CGRectMake(0, 0, engineVC_.view.frame.size.width,
                                    newFrame.size.height);
  scriptEditor_.view.frame = CGRectMake(scriptEditor_.view.frame.origin.x, 0,
                                        scriptEditor_.view.frame.size.width,
                                        newFrame.size.height);
  
  [UIView commitAnimations];
}

@end
