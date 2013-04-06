#import "EngineViewController.h"
#import "engine.h"
#import "scene.h"
#import "world.h"
#import "game_entity.h"
#import "tile_map_parse.h"
#import "ortho_physics_scene.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

char *bundlePath__;
@interface EngineViewController () {
  GLuint _program;
  
  GLuint _vertexArray;
  GLuint _vertexBuffer;
  
  Engine *engine_;
  Scene *scene_;
  
  GLKView *glkView_;
  
  Input *touchInput_;
  UILongPressGestureRecognizer *moveGesture_;
  UILongPressGestureRecognizer *jumpGesture_;
}
@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) GLKBaseEffect *effect;

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation EngineViewController
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
  
  glkView_ = [[GLKView alloc] initWithFrame:self.view.bounds];
  glkView_.context = self.context;
  glkView_.autoresizingMask = (UIViewAutoresizingFlexibleWidth |
                               UIViewAutoresizingFlexibleHeight);
  glkView_.drawableDepthFormat = GLKViewDrawableDepthFormat16;
  glkView_.delegate = self;
  self.view = glkView_;

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

- (void)setupGL {
  [EAGLContext setCurrentContext:self.context];
  
  /*
   INIT GAME ENGINE
   */
  Engine_bootstrap(&engine_, NULL);
  self.preferredFramesPerSecond = FPS;
  
  scene_ = Scene_create(engine_, OrthoPhysicsSceneProto);
  
  self.effect = [[GLKBaseEffect alloc] init];
}

- (void)tearDownGL {
  [EAGLContext setCurrentContext:self.context];
  
  // TODO: CLEANUP
  scene_->_(destroy)(scene_, engine_);
  engine_->_(destroy)(engine_);
  self.effect = nil;
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update {
  Engine_regulate(engine_);
  Input_touch(engine_->input, touchInput_);
  
  if (engine_->frame_now) {
    scene_->_(control)(scene_, engine_);
    scene_->_(update)(scene_, engine_);
    Input_reset(engine_->input);
  }
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
  scene_->camera->screen_size.w = self.view.bounds.size.width;
  scene_->camera->screen_size.h = self.view.bounds.size.height;
  
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
  Scene_load_tile_map(scene_, engine_, cNewFilePath, 1);
}

- (void)restartScene {
  Scene_restart(scene_, engine_);
}

@end
