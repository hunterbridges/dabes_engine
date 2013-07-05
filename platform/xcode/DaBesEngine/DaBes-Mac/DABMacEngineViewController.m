#import <setjmp.h>
#import <DaBes-Mac/DaBes-Mac.h>
#import "DABMacEngineViewController.h"
#import "DABMacEngineView.h"
#import "DABProjectManager.h"

NSString *kFrameEndNotification =  @"kFrameEndNotification";
NSString *kNewSceneNotification =  @"kNewSceneNotification";
NSString *kEntitySelectedNotification = @"kEntitySelectedNotification";
NSString *kEngineReadyForScriptNotification =
    @"kEngineReadyForScriptNotification";
NSString *kEngineHasScriptErrorNotification =
    @"kEngineHasScriptErrorNotification";
NSString *kEngineHasScriptPanicNotification =
    @"kEngineHasScriptPanicNotification";

void Mac_script_error_cb(const char *error) {
  NSString *str =
      [[NSString alloc] initWithCString:error encoding:NSUTF8StringEncoding];
  [[NSNotificationCenter defaultCenter]
      postNotificationName:kEngineHasScriptErrorNotification object:str];
}

static jmp_buf panic_jmp_buf;
void Mac_script_panic_cb(const char *error) {
  NSString *str =
      [[NSString alloc] initWithCString:error encoding:NSUTF8StringEncoding];
  [[NSNotificationCenter defaultCenter]
      postNotificationName:kEngineHasScriptPanicNotification object:str];
  longjmp(panic_jmp_buf, 1);
}

char *Mac_resource_path(const char *filename) {
  NSString *nsFilename = [NSString stringWithCString:filename
                                            encoding:NSUTF8StringEncoding];
  nsFilename = [@"resource/" stringByAppendingString:nsFilename];
  
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

void Mac_handle_abort_trap(int err) {
    NSLog(@"Should never hit this... ABORT TRAP!");
}

////////////////////////////////////////////////////////////////////////////////

@interface DABMacEngineViewController ()

@property (nonatomic, strong) NSTimer *updateTimer;
@property (nonatomic, assign) BOOL needsRestart;
@property (nonatomic, assign) BOOL rebootWhenReady;

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
  
  self.engine->scripting->error_callback = Mac_script_error_cb;
  self.engine->scripting->panic_callback = Mac_script_panic_cb;
  Scripting_boot(self.engine->scripting);
  
  ((DABMacEngineView *)self.view).engine = self.engine;
  [self refreshScene];
}

- (void)tearDown {
  if (!self.engine) return;
  Engine_destroy(self.engine);
  self.engine = nil;
}

- (void)reboot {
  self.rebootWhenReady = YES;
}

- (void)update {
  if (!self.engine) return;
  
  if (!setjmp(panic_jmp_buf)) {
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
    
    if (self.rebootWhenReady) {
      self.rebootWhenReady = NO;
      [self tearDown];
      [self initEngine];
    }
  } else {
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      [self tearDown];
      NSLog(@"Crashed.");
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
