#import <DaBes-Mac/DaBes-Mac.h>
#import "DABMacEngineViewController.h"
#import "DABMacEngineView.h"
#import "DABProjectManager.h"

NSString *kFrameEndNotification =  @"kFrameEndNotification";
NSString *kNewSceneNotification =  @"kNewSceneNotification";
NSString *kEntitySelectedNotification = @"kEntitySelectedNotification";
NSString *kEngineReadyForScriptNotification =
    @"kEngineReadyForScriptNotification";

const char *Mac_resource_path(const char *filename) {
  NSString *nsFilename = [NSString stringWithCString:filename
                                            encoding:NSUTF8StringEncoding];
  nsFilename = [@"resources/" stringByAppendingString:nsFilename];
  
  NSFileManager *manager = [NSFileManager defaultManager];
  NSString *bundlePath = [[[NSBundle mainBundle] resourcePath]
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

@interface DABMacEngineViewController ()

@property (nonatomic, strong) NSTimer *updateTimer;
@property (nonatomic, assign) BOOL needsRestart;

@end

char *bundlePath__;
@implementation DABMacEngineViewController

- (id)init {
  self = [super init];
  if (self) {
    bundlePath__ = (char *)[[NSString stringWithFormat:@"%@/",
                             [[NSBundle mainBundle] bundlePath]]
                   cStringUsingEncoding:NSASCIIStringEncoding];
    self.touchInput = Input_create();
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
  self.view = [[DABMacEngineView alloc] initWithFrame:CGRectZero
                                       pixelFormat:format
                                        touchInput:self.touchInput];
  [((DABMacEngineView *)self.view).openGLContext makeCurrentContext];
  [self initEngine];
  
  if (self.engine) Engine_regulate(self.engine);
  
  self.updateTimer = [NSTimer scheduledTimerWithTimeInterval:1/FPS
                                                      target:self
                                                    selector:@selector(update)
                                                    userInfo:nil
                                                     repeats:YES];
  [[NSRunLoop currentRunLoop]
      addTimer:self.updateTimer
      forMode:NSEventTrackingRunLoopMode];
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
  
  self.engine = Engine_create(Mac_resource_path, DABProjectManager_path_func, cBootScript, NULL);
  Scripting_boot(self.engine->scripting);
  ((DABMacEngineView *)self.view).engine = self.engine;
  [self refreshScene];
}

- (void)update {
  if (!self.engine) return;
  
  Engine_regulate(self.engine);
  Input_touch(self.engine->input, self.touchInput);
  Audio_stream(self.engine->audio);
  
  if (self.engine->frame_now) {
    Engine_update_easers(self.engine);
    Scene *scene = self.scene;
    if (scene) Scene_update(scene, self.engine);
    [self draw];
    Input_reset(self.engine->input);
    
    [[NSNotificationCenter defaultCenter]
        postNotificationName:kEngineReadyForScriptNotification
        object:self];
    
    Engine_frame_end(self.engine);
    [self refreshScene];
    [[NSNotificationCenter defaultCenter]
        postNotificationName:kFrameEndNotification
        object:self];
  }
}

- (void)restartCurrentScene {
  self.needsRestart = YES;
}

- (void)draw {
  [self.view setNeedsDisplay:YES];
}

- (void)refreshScene {
  Scene *old = self.scene;
  Scene *new = Engine_get_current_scene(self.engine);
  if (old == new) {
    if (self.needsRestart) {
      Scene_restart(self.scene, self.engine);
      self.needsRestart = NO;
    }
  } else {
    self.scene = new;
    ((DABMacEngineView *)self.view).scene = new;
    [[NSNotificationCenter defaultCenter]
        postNotificationName:kNewSceneNotification
        object:self];
  }
}


@end
