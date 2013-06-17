#import "EngineViewController.h"
#import "scene.h"
#import "world.h"
#import "entity.h"
#import "tile_map_parse.h"
#import "ortho_chipmunk_scene.h"

NSTimeInterval kBufferRefreshDelay = 0.01;
NSString *kEngineReadyForScriptNotification =
    @"kEngineReadyForScriptNotification";

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

char *bundlePath__;
@interface EngineViewController () {
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
@property (nonatomic, strong) NSThread *audioThread;
@property (nonatomic, assign) BOOL audioThreadFinished;

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation EngineViewController
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

  [self setupGL];
  if (!engine_) [self initEngine];
}

- (void)viewDidAppear:(BOOL)animated {
  /*
  [NSTimer scheduledTimerWithTimeInterval:2.0
                                   target:self
                                 selector:@selector(restartScene)
                                 userInfo:nil
                                  repeats:YES];
   */
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

- (void)initEngine {
  engine_ = Engine_create("scripts/boxfall/boot.lua", NULL);
  Scripting_boot(engine_->scripting);
  
  self.audioThreadFinished = NO;
  self.audioThread =
      [[NSThread alloc] initWithTarget:self
                              selector:@selector(runAudioThread:)
                                object:self];
  [self.audioThread start];
}

- (void)runAudioThread:(EngineViewController *)viewController {
  while (true) {
    Audio_stream(viewController.engine->audio);
    [NSThread sleepForTimeInterval:kBufferRefreshDelay];
    if ([NSThread currentThread].isCancelled) {
      viewController.audioThreadFinished = YES;
      return;
    }
  }
}

- (void)setupGL {
  [EAGLContext setCurrentContext:self.context];
  
  self.preferredFramesPerSecond = 60;
  self.effect = [[GLKBaseEffect alloc] init];
}

- (void)tearDownGL {
  [EAGLContext setCurrentContext:self.context];
  
  // TODO: CLEANUP
  Scene_destroy(scene_, engine_);
  engine_->_(destroy)(engine_);
  self.effect = nil;
}

- (void)fullUpdate {
  [self update];
  [self.glkView display];
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update {
  Engine_regulate(engine_);
  Input_touch(engine_->input, touchInput_);
  
  scene_ = Engine_get_current_scene(engine_);
  
  if (engine_->frame_now) {
    Engine_update_easers(engine_);
    if (scene_) scene_->_(control)(scene_, engine_);
    if (scene_) scene_->_(update)(scene_, engine_);
    Input_reset(engine_->input);
    
    [[NSNotificationCenter defaultCenter]
        postNotificationName:kEngineReadyForScriptNotification
        object:self];
    
    Engine_frame_end(engine_);
  }
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
  if (!(scene_ && scene_->started)) return;
  scene_->camera->screen_size.w = self.view.bounds.size.width;
  scene_->camera->screen_size.h = self.view.bounds.size.height;
  
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  scene_->_(render)(scene_, engine_);
#ifdef DEBUG
  Graphics_draw_debug_text(engine_->graphics, engine_->frame_ticks);
#endif
}

- (void)didEnterBackground {
  Engine_pause_time(engine_);
}

- (void)willEnterForeground {
  Engine_resume_time(engine_);
}

- (void)injectMapFromPath:(NSString *)newFilePath {
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
  Scene_restart(scene_, engine_);
}

- (void)reboot {
  [self addObserver:self
         forKeyPath:@"audioThreadFinished"
            options:NSKeyValueObservingOptionNew
            context:NULL];
  [self.audioThread cancel];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context {
  if (object == self && [keyPath isEqualToString:@"audioThreadFinished"] &&
          [[change objectForKey:NSKeyValueChangeNewKey] boolValue]) {
      [self removeObserver:self forKeyPath:@"audioThreadFinished"];
      double delayInSeconds = 0.1;
      dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
      dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
          Engine_destroy(engine_);
          self.audioThread = nil;
          [self initEngine];
      });
      return;
  }
  [super observeValueForKeyPath:keyPath ofObject:object change:change context:context];
}

@end
