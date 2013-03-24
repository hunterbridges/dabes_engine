#import "InterfaceViewController.h"
#import "EngineViewController.h"

@interface InterfaceViewController () {
  EngineViewController *engineVC_;
  BOOL showingMenu_;
  UISwipeGestureRecognizer *menuSwipe_;
  UIScrollView *debugMenuView_;
  float menuHeight_;
  int pages_;
}

@end

@implementation InterfaceViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  if (touchInput_ == NULL) {
    touchInput_ = NEW(Input, "Touch input");
  }
  
  self.view.backgroundColor = [UIColor colorWithWhite:0.65 alpha:1];
  
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
  debugMenuView_.delaysContentTouches = NO;
  [self.view addSubview:debugMenuView_];
  
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
  
  engineVC_ = [[EngineViewController alloc] initWithTouchInput:touchInput_];
  [self addChildViewController:engineVC_];
  //[self.view addSubview:engineVC_.view];
  
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
  
  /* LAYOUT CONSTRAINT BULLSHIT */
  NSDictionary *views =
      NSDictionaryOfVariableBindings(camLabel, envLabel, rotLabel,
                                     rotSlider, zoomLabel, zoomSlider,
                                     debugMenuView_, resetCamButton);
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
  
  NSArray *col =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"V:|-[camLabel(20)]-[rotLabel(==camLabel)]-[zoomLabel(==camLabel)]"
          @"-(40)-[envLabel]"
          options:NSLayoutFormatAlignAllLeft
          metrics:nil
          views:views];
  [debugMenuView_ addConstraints:col];
  
  NSArray *row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[rotLabel(50)]-[rotSlider(<=camLabel)]-[resetCamButton(60)]-|"
          options:NSLayoutFormatAlignAllTop
          metrics:nil
          views:views];
  [debugMenuView_ addConstraints:row];
  
  row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[zoomLabel(50)]-[zoomSlider(==rotSlider)]-[resetCamButton(60)]-|"
          options:NSLayoutFormatAlignAllBottom
          metrics:nil
          views:views];
  [debugMenuView_ addConstraints:row];
}

- (void)dealloc {
  Input_destroy(touchInput_);
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];
}

- (void)handleSwipe:(UISwipeGestureRecognizer *)gesture {
  if (gesture.direction == UISwipeGestureRecognizerDirectionDown) {
    [self showMenu];
  } else if (gesture.direction == UISwipeGestureRecognizerDirectionUp) {
    [self hideMenu];
    
  }
}

- (void)viewDidLayoutSubviews {
  debugMenuView_.contentSize = CGSizeMake(self.view.bounds.size.width,
                                         menuHeight_ * pages_);
}

- (void)scrollViewDidScroll:(UIScrollView *)scrollView {
  
}

- (void)showMenu {
  if (showingMenu_) return;
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

- (void)willEnterForeground {
  if ([engineVC_.view superview]) [engineVC_ willEnterForeground];
}

#pragma mark Debug controls
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

@end
