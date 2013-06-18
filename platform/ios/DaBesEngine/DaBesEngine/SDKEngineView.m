#import "SDKEngineView.h"
#import "scene.h"

@interface SDKEngineView ()

@end

@implementation SDKEngineView

- (void)update {
  
}

- (void)drawRect:(NSRect)dirtyRect
{
  // Drawing code here.
  [self.openGLContext makeCurrentContext];
  if (!self.engine) return;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  Scene *scene = Engine_get_current_scene(self.engine);
  if (!scene) return;
  scene->_(render)(scene, self.engine);
  [self.openGLContext flushBuffer];
}

@end
