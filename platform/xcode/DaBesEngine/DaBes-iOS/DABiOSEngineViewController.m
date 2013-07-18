#import <DaBes-iOS/DaBes-iOS.h>
#import "DABiOSEngineViewController.h"
#import "DABProjectManager.h"

NSTimeInterval kBufferRefreshDelay = 0.01;
NSString *kFrameEndNotification =  @"kFrameEndNotification";
NSString *kEngineReadyForScriptNotification =
    @"kEngineReadyForScriptNotification";
NSString *kEntitySelectedNotification = @"kEntitySelectedNotification";

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
@interface DABiOSEngineViewController () {
  GLuint _program;
  
  GLuint _vertexArray;
  GLuint _vertexBuffer;
  
  Input *touchInput_;
  UILongPressGestureRecognizer *moveGesture_;
  UILongPressGestureRecognizer *jumpGesture_;
}

@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) GLKBaseEffect *effect;
@property (nonatomic, strong) GLKView *glkView;
@property (nonatomic, assign) Engine *engine;
@property (nonatomic, assign) Scene *scene;
@property (nonatomic, strong) UITapGestureRecognizer *tapGesture;

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation DABiOSEngineViewController
@synthesize engine = engine_;
@synthesize scene = scene_;

- (id)initWithTouchInput:(Input *)input {
  self = [super init];
  if (self) {
    touchInput_ = input;
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
  
  moveGesture_ =
      [[UILongPressGestureRecognizer alloc]
          initWithTarget:self
          action:@selector(handleMove:)];
  moveGesture_.minimumPressDuration=0.05;
  moveGesture_.delegate = self;
  [self.view addGestureRecognizer:moveGesture_];

  jumpGesture_ =
      [[UILongPressGestureRecognizer alloc]
           initWithTarget:self
           action:@selector(handleJump:)];
  jumpGesture_.numberOfTouchesRequired = 2;
  jumpGesture_.minimumPressDuration=0.05;
  jumpGesture_.delegate = self;
  [self.view addGestureRecognizer:jumpGesture_];
  
  self.tapGesture =
      [[UITapGestureRecognizer alloc]
          initWithTarget:self
          action:@selector(handleTap:)];
  self.tapGesture.numberOfTouchesRequired = 1;
  self.tapGesture.delegate = self;
  [self.view addGestureRecognizer:self.tapGesture];

  [self setupGL];
  if (!engine_) [self initEngine];
}

- (BOOL)gestureRecognizer:(UIGestureRecognizer *)gestureRecognizer
    shouldRecognizeSimultaneouslyWithGestureRecognizer:
        (UIGestureRecognizer *)otherGestureRecognizer {
  return YES;
}

- (void)handleMove:(UILongPressGestureRecognizer *)gesture {
  CGRect screen = self.view.bounds;
  CGPoint loc = [gesture locationOfTouch:0 inView:self.view];
  BOOL ended = gesture.state == UIGestureRecognizerStateEnded;
  
  Controller *controller = touchInput_->controllers[0];
  Controller_dpad_direction direction = CONTROLLER_DPAD_NONE;
  
  if (loc.x <= screen.size.width / 2.0) {
    if (controller->dpad & CONTROLLER_DPAD_RIGHT)
      controller->dpad &= ~(CONTROLLER_DPAD_RIGHT);
    direction = CONTROLLER_DPAD_LEFT;
  } else {
    if (controller->dpad & CONTROLLER_DPAD_LEFT)
      controller->dpad &= ~(CONTROLLER_DPAD_LEFT);
    direction = CONTROLLER_DPAD_RIGHT;
  }
  
  if (ended) {
      controller->dpad &= ~(direction);
  } else {
      controller->dpad |= direction;
  }
}

- (void)handleJump:(UILongPressGestureRecognizer *)gesture {
  Controller *controller = touchInput_->controllers[0];
  BOOL ended = gesture.state == UIGestureRecognizerStateEnded;
  if (ended) {
      controller->jump = 0;
  } else {
      controller->jump = 1;
  }
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

- (void)initEngine {
  NSString *bootScript = self.bootScript;
  if (!bootScript) return;
  [DABProjectManager sharedInstance].projectPath = self.projectPath;
  const char *cBootScript =
      [bootScript cStringUsingEncoding:NSUTF8StringEncoding];
  engine_ = Engine_create(iOS_resource_path, DABProjectManager_path_func, cBootScript, NULL);
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

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update {
  if (!self.engine) return;
  
  Engine_regulate(engine_);
  Audio_stream(self.engine->audio, self.engine);
  Input_touch(engine_->input, touchInput_);
  
  scene_ = Engine_get_current_scene(engine_);
  
  if (engine_->frame_now) {
    Engine_update_easers(engine_);
    if (scene_) Scene_update(scene_, engine_);
    Input_reset(engine_->input);
    
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
