#import "InterfaceViewController.h"
#import "EngineViewController.h"

@interface InterfaceViewController () {
  EngineViewController *engineVC_;
  BOOL showingMenu_;
  UISwipeGestureRecognizer *menuSwipe_;
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
  
  UILabel *camLabel = [[UILabel alloc] init];
  camLabel.backgroundColor = [UIColor clearColor];
  camLabel.textColor = [UIColor whiteColor];
  camLabel.textAlignment = NSTextAlignmentLeft;
  camLabel.text = @"Camera";
  camLabel.font = [UIFont boldSystemFontOfSize:20];
  camLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:camLabel];
  
  UILabel *envLabel = [[UILabel alloc] init];
  envLabel.backgroundColor = [UIColor clearColor];
  envLabel.textColor = [UIColor whiteColor];
  envLabel.textAlignment = NSTextAlignmentLeft;
  envLabel.text = @"Environment";
  envLabel.font = [UIFont boldSystemFontOfSize:20];
  envLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:envLabel];
  
  UILabel *rotLabel = [[UILabel alloc] init];
  rotLabel.backgroundColor = [UIColor clearColor];
  rotLabel.textColor = [UIColor whiteColor];
  rotLabel.textAlignment = NSTextAlignmentLeft;
  rotLabel.text = @"Rot";
  rotLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:rotLabel];
  
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
  [self.view addSubview:rotSlider];
  
  UILabel *zoomLabel = [[UILabel alloc] init];
  zoomLabel.backgroundColor = [UIColor clearColor];
  zoomLabel.textColor = [UIColor whiteColor];
  zoomLabel.textAlignment = NSTextAlignmentLeft;
  zoomLabel.text = @"Zoom";
  zoomLabel.translatesAutoresizingMaskIntoConstraints = NO;
  [self.view addSubview:zoomLabel];
  
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
  [self.view addSubview:zoomSlider];
  
  engineVC_ = [[EngineViewController alloc] initWithTouchInput:touchInput_];
  [self addChildViewController:engineVC_];
  [self.view addSubview:engineVC_.view];
  
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
                                     rotSlider, zoomLabel, zoomSlider);
  NSArray *headers =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[camLabel]-[envLabel(==camLabel)]-|"
          options:NSLayoutFormatAlignAllTop
          metrics:nil
          views:views];
  [self.view addConstraints:headers];
  
  NSArray *col1 =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"V:|-[camLabel(20)]-[rotLabel(==camLabel)]-[zoomLabel(==camLabel)]"
          options:NSLayoutFormatAlignAllLeft
          metrics:nil
          views:views];
  [self.view addConstraints:col1];
  
  NSArray *row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[rotLabel(50)]-[rotSlider(<=camLabel)]"
          options:NSLayoutFormatAlignAllTop
          metrics:nil
          views:views];
  NSLayoutConstraint *slider =
    [NSLayoutConstraint constraintWithItem:rotSlider
                                 attribute:NSLayoutAttributeRight
                                 relatedBy:NSLayoutRelationEqual
                                    toItem:camLabel
                                 attribute:NSLayoutAttributeRight
                                multiplier:1.0
                                  constant:0];
  [self.view addConstraints:row];
  [self.view addConstraint:slider];
  
  row =
      [NSLayoutConstraint constraintsWithVisualFormat:
          @"|-[zoomLabel(50)]-[zoomSlider(==rotSlider)]"
          options:NSLayoutFormatAlignAllTop
          metrics:nil
          views:views];
  [self.view addConstraints:row];
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

- (void)showMenu {
  if (showingMenu_) return;
  [UIView animateWithDuration:0.5
       animations:^{
         engineVC_.view.frame = CGRectMake(0, 120,
                                           self.view.bounds.size.width,
                                           self.view.bounds.size.height - 120);
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


@end
