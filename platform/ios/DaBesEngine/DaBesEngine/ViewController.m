#import "ViewController.h"
#import "engine.h"
#import "scene.h"
#import "world.h"
#import "game_entity.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

char *bundlePath__;
@interface ViewController () {
    GLuint _program;
    
    GLKMatrix4 _modelViewProjectionMatrix;
    GLKMatrix3 _normalMatrix;
    float _rotation;
    
    GLuint _vertexArray;
    GLuint _vertexBuffer;
  
  Engine *engine_;
  Scene *scene_;
  World *world_;
  
  Controller *touchController_;
  UILongPressGestureRecognizer *moveGesture_;
  UILongPressGestureRecognizer *jumpGesture_;
}
@property (strong, nonatomic) EAGLContext *context;
@property (strong, nonatomic) GLKBaseEffect *effect;

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation ViewController

- (void)viewDidLoad {
  [super viewDidLoad];
  bundlePath__ = (char *)[[NSString stringWithFormat:@"%@/",
                           [[NSBundle mainBundle] bundlePath]]
                 cStringUsingEncoding:NSASCIIStringEncoding];

  self.context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];

  if (!self.context) {
      NSLog(@"Failed to create ES context");
  }
  
  GLKView *view = (GLKView *)self.view;
  view.context = self.context;
  view.drawableDepthFormat = GLKViewDrawableDepthFormat16;
  //view.drawableMultisample = GLKViewDrawableMultisample4X;

  touchController_ = calloc(1, sizeof(Controller));

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
  Controller_dpad_direction direction = CONTROLLER_DPAD_NONE;
  
  if (loc.x <= screen.size.width / 2.0) {
    if (touchController_->dpad & CONTROLLER_DPAD_RIGHT)
      touchController_->dpad &= ~(CONTROLLER_DPAD_RIGHT);
    direction = CONTROLLER_DPAD_LEFT;
  } else {
    if (touchController_->dpad & CONTROLLER_DPAD_LEFT)
      touchController_->dpad &= ~(CONTROLLER_DPAD_LEFT);
    direction = CONTROLLER_DPAD_RIGHT;
  }
  
  if (ended) {
      touchController_->dpad &= ~(direction);
  } else {
      touchController_->dpad |= direction;
  }
}

- (void)handleJump:(UILongPressGestureRecognizer *)gesture {
  BOOL ended = gesture.state == UIGestureRecognizerStateEnded;
  if (ended) {
      touchController_->jump = 0;
  } else {
      touchController_->jump = 1;
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
  
  scene_ = NEW(Scene, "The game");
  world_ = Scene_create_world(scene_, engine_->physics);
  
  GameEntity_assign_controller(scene_->entities->first->value,
          engine_->input->controllers[0]);
  
  self.effect = [[GLKBaseEffect alloc] init];
}

- (void)tearDownGL {
  [EAGLContext setCurrentContext:self.context];
  
  // TODO: CLEANUP
  scene_->_(destroy)(scene_);
  engine_->_(destroy)(engine_);
  self.effect = nil;
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
  /* Physics, camera, input */
  Engine_regulate(engine_);
  //Input_poll(engine_->input);
  Input_touch(engine_->input, touchController_, 0);
  
  if (engine_->frame_now) {
    Scene_control(scene_, engine_->input);

    World_solve(engine_->physics, world_, engine_->frame_ticks);
    Input_reset(engine_->input);
  }
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect {
  Scene_render(scene_, engine_);
#ifdef DEBUG
  Graphics_draw_debug_text(engine_->graphics, engine_->frame_ticks);
#endif
}

@end
