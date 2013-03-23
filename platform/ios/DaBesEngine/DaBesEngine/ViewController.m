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
  
    [self setupGL];
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
  Input_poll(engine_->input);
  
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
