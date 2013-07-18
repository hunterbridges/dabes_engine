#import "InterfaceViewController.h"

#import <AssetsLibrary/AssetsLibrary.h>
#import <QuartzCore/QuartzCore.h>

#import "SDKiOSProjectEngineViewController.h"
#import "DirBrowserViewController.h"
#import "DebugMenuViewController.h"
#import "ScriptEditorViewController.h"
#import "CodeEditorView.h"

@interface InterfaceViewController () <UIGestureRecognizerDelegate> {
    Input *_touchInput;
}

@property (nonatomic, strong) SDKiOSProjectEngineViewController *engineVC;
@property (nonatomic, strong) UINavigationController *scriptEditor;
@property (nonatomic, strong) DebugMenuViewController *debugMenu;
@property (nonatomic, strong) UISwipeGestureRecognizer *menuSwipe;

@property (nonatomic, assign) BOOL showingMenu;
@property (nonatomic, assign) BOOL showingScriptEditor;

@end

@implementation InterfaceViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  if (_touchInput == NULL) {
    _touchInput = Input_create();
  }
  
  self.view.backgroundColor = [UIColor colorWithWhite:0.65 alpha:1];
  
  self.engineVC = [[SDKiOSProjectEngineViewController alloc] initWithTouchInput:_touchInput];
  self.debugMenu =
      [[DebugMenuViewController alloc] initWithNibName:@"DebugMenu" bundle:nil];
  self.debugMenu.engineVC = self.engineVC;
  self.debugMenu.touchInput = _touchInput;
  self.debugMenu.view.frame = CGRectMake(0, 0, self.view.bounds.size.width,
                                         self.debugMenu.height);
  
  [self addChildViewController:self.debugMenu];
  [self.view addSubview:self.debugMenu.view];
  
  [self addChildViewController:self.engineVC];
  [self.view addSubview:self.engineVC.view];
  
  NSString *path = [[[NSBundle mainBundle] bundlePath] stringByAppendingPathComponent:@"demo/boxfall"];
  SDKiOSProjectEngineViewController *engineVC = self.engineVC;
  DirBrowserViewController *scriptBrowser =
      [[DirBrowserViewController alloc] initWithPath:path
          withFileSelectionBlock:^(DirBrowserViewController *vc, NSString *item) {
            ScriptEditorViewController *editor =
                [[ScriptEditorViewController alloc] initWithEngineVC:engineVC
                                                            withPath:item];
            [vc.navigationController pushViewController:editor animated:YES];
          }];
  self.scriptEditor =
      [[UINavigationController alloc] initWithRootViewController:scriptBrowser];
  self.scriptEditor.navigationBar.tintColor = [UIColor blackColor];
  self.scriptEditor.view.clipsToBounds = YES;
  
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

- (void)viewWillDisappear:(BOOL)animated {
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)dealloc {
  Input_destroy(_touchInput);
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
}

- (void)viewDidLayoutSubviews {
  self.debugMenu.view.frame = CGRectMake(0, 0, self.view.bounds.size.width,
                                         self.debugMenu.height);
  self.debugMenu.scrollView.contentSize = CGSizeMake(self.view.bounds.size.width,
                                                     self.debugMenu.height *
                                                         self.debugMenu.pages);
  if (self.showingScriptEditor) {
     self.scriptEditor.view.frame = CGRectMake(self.view.bounds.size.width / 2,
                                               0,
                                               self.view.bounds.size.width / 2,
                                               self.view.bounds.size.height);
     self.engineVC.view.frame = CGRectMake(0, 0,
                                           self.view.bounds.size.width / 2,
                                           self.view.bounds.size.height);
  }
}

#pragma mark - Public

- (void)didEnterBackground {
  if ([self.engineVC.view superview]) [self.engineVC didEnterBackground];
}

- (void)willEnterForeground {
  if ([self.engineVC.view superview]) [self.engineVC willEnterForeground];
}

- (void)injectMapFromPath:(NSString *)newFilePath {
  [self.engineVC injectMapFromPath:newFilePath];
}

- (void)log:(NSString *)fmt arguments:(va_list)arguments {
  [self.debugMenu log:fmt arguments:arguments];
  [self showMenu];
}

#pragma mark - Private

- (void)showMenu {
  if (self.showingMenu) return;
  if (self.showingScriptEditor) return;
  [self.debugMenu viewWillAppear:YES];
  [UIView animateWithDuration:0.5
       animations:^{
         self.engineVC.view.frame = CGRectMake(0, self.debugMenu.height,
                                               self.view.bounds.size.width,
                                               self.view.bounds.size.height
                                                   - self.debugMenu.height);
       }
       completion:^(BOOL finished) {
         [self.debugMenu viewDidAppear:YES];
       }];
  self.showingMenu = YES;
}

- (void)hideMenu {
  if (!self.showingMenu) return;
  [self.debugMenu viewWillDisappear:YES];
  [UIView animateWithDuration:0.5
       animations:^{
         self.engineVC.view.frame = CGRectMake(0, 0,
                                               self.view.bounds.size.width,
                                               self.view.bounds.size.height);
       }
       completion:^(BOOL finished) {
         [self.debugMenu viewDidDisappear:YES];
       }];
  self.showingMenu = NO;
}

- (void)showScriptEditor {
  if (self.showingMenu) return;
  if (self.showingScriptEditor) return;
  self.showingScriptEditor = YES;
  self.scriptEditor.view.frame = CGRectMake(self.view.bounds.size.width,
                                            0,
                                            self.view.bounds.size.width / 2,
                                            self.view.bounds.size.height);
  [self addChildViewController:self.scriptEditor];
  [self.view addSubview:self.scriptEditor.view];
  [UIView animateWithDuration:0.5
       animations:^{
         self.scriptEditor.view.frame = CGRectMake(self.view.bounds.size.width / 2,
                                                   0,
                                                   self.view.bounds.size.width / 2,
                                                   self.view.bounds.size.height);
         self.engineVC.view.frame = CGRectMake(0, 0,
                                               self.view.bounds.size.width / 2,
                                               self.view.bounds.size.height);
       }];
  
}

- (void)hideScriptEditor {
  if (!self.showingScriptEditor) return;

  [UIView animateWithDuration:0.5
       animations:^{
         self.scriptEditor.view.frame = CGRectMake(self.view.bounds.size.width,
                                                   0,
                                                   self.view.bounds.size.width / 2,
                                                   self.view.bounds.size.height);
         self.engineVC.view.frame = CGRectMake(0, 0,
                                               self.view.bounds.size.width,
                                               self.view.bounds.size.height);
       }
       completion:^(BOOL finished) {
         [self.scriptEditor removeFromParentViewController];
         [self.scriptEditor.view removeFromSuperview];
         self.showingScriptEditor = NO;
       }];
}

- (void)buildDebugMenu {
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
  self.engineVC.view.frame = CGRectMake(0, 0, self.engineVC.view.frame.size.width,
                                    newFrame.size.height);
  self.scriptEditor.view.frame = CGRectMake(self.scriptEditor.view.frame.origin.x, 0,
                                            self.scriptEditor.view.frame.size.width,
                                            newFrame.size.height);
  
  [UIView commitAnimations];
}

@end
