#import <DaBes-iOS/DaBes-iOS.h>
#import "DABiOSEngineViewController.h"
#import "DABProjectManager.h"
#import "DABPlatformerControllerOverlayView.h"
#import "DABTouchpadControllerOverlayView.h"

NSTimeInterval kBufferRefreshDelay = 0.01;
NSString *kFrameEndNotification =  @"kFrameEndNotification";
NSString *kEngineReadyForScriptNotification =
    @"kEngineReadyForScriptNotification";
NSString *kEntitySelectedNotification = @"kEntitySelectedNotification";
NSString *kInputChangedPreferredStyleNotification =
    @"kInputChangedPreferredStyleNotification";

char *iOS_resource_path(const char *filename) {
  NSString *nsFilename = [NSString stringWithCString:filename
                                            encoding:NSUTF8StringEncoding];
  nsFilename = [@"resource/" stringByAppendingString:nsFilename];
  
  NSFileManager *manager = [NSFileManager defaultManager];
  NSString *bundlePath = [[[NSBundle mainBundle] bundlePath]
                          stringByAppendingPathComponent:nsFilename];
  NSArray *docsPath =
  NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                      NSUserDomainMask, YES);
  NSString *docsFile = [[docsPath objectAtIndex:0]
                        stringByAppendingPathComponent:nsFilename];
  
  const char *cFullpath;
  if ([manager fileExistsAtPath:docsFile]) {
    cFullpath = [docsFile cStringUsingEncoding:NSUTF8StringEncoding];
  } else if ([manager fileExistsAtPath:bundlePath]) {
    cFullpath = [bundlePath cStringUsingEncoding:NSUTF8StringEncoding];
  } else {
    cFullpath = [bundlePath cStringUsingEncoding:NSUTF8StringEncoding];
  }
  
  char *cpy = calloc(strlen(cFullpath) + 1, sizeof(char));
  strcpy(cpy, cFullpath);
  return cpy;
}

////////////////////////////////////////////////////////////////////////////////

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

char *bundlePath__;
@interface DABiOSEngineViewController ()

@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) GLKBaseEffect *effect;
@property (nonatomic, strong) GLKView *glkView;
@property (nonatomic, assign) Engine *engine;
@property (nonatomic, assign) Scene *scene;
@property (nonatomic, strong) UITapGestureRecognizer *tapGesture;

@property (nonatomic, assign) Input *touchInput;
@property (nonatomic, assign) InputStyle currentInputStyle;
@property (nonatomic, strong) UIView *currentInputView;
@property (nonatomic, strong) NSMutableDictionary *inputViews;

- (void)setupGL;
- (void)tearDownGL;
- (void)primeThread;
  
@end

@implementation DABiOSEngineViewController
@synthesize engine = engine_;
@synthesize scene = scene_;

- (id)initWithTouchInput:(Input *)input {
  self = [super init];
  if (self) {
    self.touchInput = input;
      
    [[NSNotificationCenter defaultCenter]
        addObserver:self
        selector:@selector(handleChangePreferredInputStyle:)
        name:kInputChangedPreferredStyleNotification
        object:nil];
  }
  return self;
}

- (void)viewDidLoad {
  [super viewDidLoad];
  bundlePath__ = (char *)[[NSString stringWithFormat:@"%@/",
                           [[NSBundle mainBundle] bundlePath]]
                 cStringUsingEncoding:NSASCIIStringEncoding];

  self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
  if (!self.context) {
      NSLog(@"Failed to create ES context");
  }
  
  self.glkView = [[GLKView alloc] initWithFrame:self.view.bounds];
  self.glkView.context = self.context;
  self.glkView.autoresizingMask = (UIViewAutoresizingFlexibleWidth |
                                   UIViewAutoresizingFlexibleHeight);
  self.glkView.drawableDepthFormat = GLKViewDrawableDepthFormat16;
  self.glkView.delegate = self;
  self.view = self.glkView;
  self.view.opaque = YES;
  CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.view.layer;
  eaglLayer.drawableProperties = @{
    kEAGLDrawablePropertyRetainedBacking: @(NO),
    kEAGLDrawablePropertyColorFormat: kEAGLColorFormatRGB565
  };
  
  self.tapGesture =
      [[UITapGestureRecognizer alloc]
          initWithTarget:self
          action:@selector(handleTap:)];
  self.tapGesture.numberOfTouchesRequired = 1;
  self.tapGesture.delegate = self;
  [self.view addGestureRecognizer:self.tapGesture];

  NSThread *primerThread = [[NSThread alloc] initWithTarget:self
                                                   selector:@selector(primeThread)
                                                     object:nil];
  [primerThread start];
  
  [self setupGL];
  if (!engine_) [self initEngine];
}

- (void)primeThread {
  NSLog(@"Priming engine for multithreading...");
  if ([NSThread isMultiThreaded]) {
    NSLog(@"Engine is multithreaded");
  }
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer
    shouldRecognizeSimultaneouslyWithGestureRecognizer:
        (UIGestureRecognizer *)otherGestureRecognizer {
  return YES;
}

- (void)handleTap:(UITapGestureRecognizer *)gesture {
  Scene *scene = self.scene;
  if (!scene) return;
  
  CGPoint loc = [gesture locationInView:self.view];
  VPoint screen_point = {loc.x, loc.y};
  int selected = Scene_select_entities_at(scene, screen_point);
  if (selected) {
    [[NSNotificationCenter defaultCenter]
        postNotificationName:kEntitySelectedNotification
        object:self
        userInfo:nil];
  }
}

- (void)dealloc {
    [self tearDownGL];
    
    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
}

- (void)didReceiveMemoryWarning {
  [super didReceiveMemoryWarning];

  if ([self isViewLoaded] && ([[self view] window] == nil)) {
      self.view = nil;
      
      [self tearDownGL];
      
      if ([EAGLContext currentContext] == self.context) {
          [EAGLContext setCurrentContext:nil];
      }
      self.context = nil;
  }
}

- (NSString *)bootScript {
  return nil;
}

- (NSString *)projectPath {
  return nil;
}

static void iOS_input_change_preferred_style_cb(Input *input,
                                                InputStyle old_style,
                                                InputStyle new_style)
{
    [[NSNotificationCenter defaultCenter]
        postNotificationName:kInputChangedPreferredStyleNotification
        object:@{@"old": @(old_style), @"new": @(new_style)}];
}

- (void)initEngine {
  NSString *bootScript = self.bootScript;
  if (!bootScript) return;
  [DABProjectManager sharedInstance].projectPath = self.projectPath;
  const char *cBootScript =
      [bootScript cStringUsingEncoding:NSUTF8StringEncoding];
  engine_ = Engine_create(iOS_resource_path, DABProjectManager_path_func,
                          STDIOConsoleProto, cBootScript, NULL);
  engine_->input->change_preferred_style_cb =
      iOS_input_change_preferred_style_cb;
    
  self.currentInputStyle = INPUT_STYLE_LEFT_RIGHT;
    
  Scripting_boot(engine_->scripting);
}

- (void)setupGL {
  [EAGLContext setCurrentContext:self.context];
  
  self.preferredFramesPerSecond = 60;
  self.effect = [[GLKBaseEffect alloc] init];
}

- (void)tearDownGL {
  [EAGLContext setCurrentContext:self.context];
  
  // TODO: CLEANUP
  if (self.engine) {
    Scene_destroy(scene_, engine_);
    Engine_destroy(engine_);
  }
  self.effect = nil;
}

- (void)fullUpdate {
  [self update];
  [self.glkView display];
}

- (UIView *)inputViewForInputStyle:(InputStyle)inputStyle
{
    UIView *inputView = self.inputViews[@(inputStyle)];
    if (inputView) {
        return inputView;
    }
    
    switch (inputStyle) {
        case INPUT_STYLE_LEFT_RIGHT:
            inputView =
                [[DABPlatformerControllerOverlayView alloc] initWithFrame:self.view.bounds];
            ((DABPlatformerControllerOverlayView *)inputView).touchInput =
                self.touchInput;
            break;
        
        case INPUT_STYLE_TOUCHPAD:
            inputView =
                [[DABTouchpadControllerOverlayView alloc] initWithFrame:self.view.bounds];
            ((DABTouchpadControllerOverlayView *)inputView).touchInput =
                self.touchInput;
            break;
            
        default:
            break;
    }

    inputView.autoresizingMask = (UIViewAutoresizingFlexibleWidth |
                                  UIViewAutoresizingFlexibleHeight);

    if (inputView) {
        self.inputViews[@(inputStyle)] = inputView;
    }

    return inputView;
}

- (void)setCurrentInputStyle:(InputStyle)currentInputStyle
{
    _currentInputStyle = currentInputStyle;
    
    [self.currentInputView removeFromSuperview];
    self.currentInputView = [self inputViewForInputStyle:currentInputStyle];
    [self.view addSubview:self.currentInputView];
}

#pragma mark - Notification Handlers

- (void)handleChangePreferredInputStyle:(NSNotification *)notif
{
    NSDictionary *obj = notif.object;
    // InputStyle old_style = [obj[@"old"] intValue];
    InputStyle new_style = [obj[@"new"] intValue];
    
    self.currentInputStyle = new_style;
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update {
  if (!self.engine) return;
  
  Engine_regulate(engine_);
  Input_touch(engine_->input, self.touchInput);
  
  scene_ = Engine_get_current_scene(engine_);
  
  if (engine_->frame_now) {
    Engine_update_easers(engine_);
    if (scene_) Scene_update(scene_, engine_);
    Input_reset(engine_->input);
    Controller_reset_touches(self.touchInput->controllers[0]);
    Audio_sweep(self.engine->audio, self.engine);
    
    [[NSNotificationCenter defaultCenter]
        postNotificationName:kEngineReadyForScriptNotification
        object:self];
    
    Engine_frame_end(engine_);
    [[NSNotificationCenter defaultCenter]
        postNotificationName:kFrameEndNotification
        object:self];
  }
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
  if (!(scene_ && scene_->started)) return;
  scene_->camera->screen_size.w = self.view.bounds.size.width;
  scene_->camera->screen_size.h = self.view.bounds.size.height;
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  Scene_render(scene_, engine_);
}

- (void)didEnterBackground {
  if (!self.engine) return;
  Engine_pause_time(engine_);
}

- (void)willEnterForeground {
  if (!self.engine) return;
  Engine_resume_time(engine_);
}

- (void)injectMapFromPath:(NSString *)newFilePath {
  if (!self.engine) return;
  CFStringRef cfNewFilePath = (__bridge CFStringRef)newFilePath;
  int strlen = CFStringGetLength(cfNewFilePath);
  int maxSize = CFStringGetMaximumSizeForEncoding(strlen,
                                                  kCFStringEncodingUTF8);
  char *cNewFilePath = calloc(1, sizeof(char) * maxSize);
  CFStringGetCString(cfNewFilePath, cNewFilePath, maxSize,
                     kCFStringEncodingUTF8);
  printf("%s\n", cNewFilePath);
  Engine_log("Injecting map <%@>", [newFilePath lastPathComponent]);
  Scene_load_tile_map(scene_, engine_, cNewFilePath, 1,
                      scene_->tile_map->meters_per_tile);
}

- (void)restartScene {
  if (!self.engine) return;
  Scene_restart(scene_, engine_);
}

- (void)reboot {
  if (!self.engine) return;
  Engine_destroy(engine_);
  self.engine = NULL;
  [self initEngine];
}

@end
