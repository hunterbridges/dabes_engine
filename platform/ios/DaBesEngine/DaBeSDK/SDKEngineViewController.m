#import "SDKEngineViewController.h"
#import "SDKEngineView.h"
#import "engine.h"
#import "scene.h"
#import "scripting.h"

@interface SDKEngineViewController ()

@property (nonatomic, assign) Engine *engine;
@property (nonatomic, readonly) Scene *scene;
@property (nonatomic, strong) NSTimer *updateTimer;

@end

char *bundlePath__;
@implementation SDKEngineViewController

- (id)init {
  self = [super init];
  if (self) {
    bundlePath__ = (char *)[[NSString stringWithFormat:@"%@/",
                             [[NSBundle mainBundle] bundlePath]]
                   cStringUsingEncoding:NSASCIIStringEncoding];
  }
  
  return self;
}

- (void)loadView {
  NSOpenGLPixelFormatAttribute attr[] = {
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAAccelerated,
    NSOpenGLPFAColorSize, 32,
    NSOpenGLPFADepthSize, 16,
    0
  };
  NSOpenGLPixelFormat *format =
      [[NSOpenGLPixelFormat alloc] initWithAttributes:attr];
  self.view = [[SDKEngineView alloc] initWithFrame:CGRectZero
                                       pixelFormat:format];
  [((SDKEngineView *)self.view).openGLContext makeCurrentContext];
  [self initEngine];
  Engine_regulate(self.engine);
  self.updateTimer = [NSTimer scheduledTimerWithTimeInterval:1/FPS
                                                      target:self
                                                    selector:@selector(update)
                                                    userInfo:nil
                                                     repeats:YES];
}

- (void)initEngine {
  self.engine = Engine_create("scripts/boxfall/boot.lua", NULL);
  Scripting_boot(self.engine->scripting);
  ((SDKEngineView *)self.view).engine = self.engine;
}

- (void)update {
  Engine_regulate(self.engine);
  if (self.engine->frame_now) {
    Engine_update_easers(self.engine);
    Scene *scene = self.scene;
    if (scene) scene->_(control)(scene, self.engine);
    if (scene) scene->_(update)(scene, self.engine);
    [self draw];
    Input_reset(self.engine->input);
    
    /*
    [[NSNotificationCenter defaultCenter]
        postNotificationName:kEngineReadyForScriptNotification
        object:self];
     */
    
    Engine_frame_end(self.engine);
  }
}

- (void)draw {
  [self.view setNeedsDisplay:YES];
}

- (Scene *)scene {
  if (!self.engine) return NULL;
  return Engine_get_current_scene(self.engine);
}

@end
