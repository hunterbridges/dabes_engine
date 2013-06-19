#import "SDKEngineView.h"
#import "scene.h"
#import "input.h"
#import "controller.h"

@interface SDKEngineView ()

@property (nonatomic, assign) Input *touchInput;
@end

@implementation SDKEngineView

- (id)initWithFrame:(NSRect)frameRect
        pixelFormat:(NSOpenGLPixelFormat *)format
         touchInput:(Input *)touchInput {
  self = [super initWithFrame:frameRect pixelFormat:format];
  if (self) {
    self.touchInput = touchInput;
  }
  return self;
}

- (void)update {
  
}

- (BOOL)acceptsFirstResponder {
  return YES;
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

- (void)keyDown:(NSEvent *)theEvent {
  // Arrow keys are associated with the numeric keypad
  NSString *keys = [theEvent charactersIgnoringModifiers];
  unichar keyChar = 0;
  if ( [keys length] == 0 ) return;
  keyChar = [keys characterAtIndex:0];
  if ([theEvent modifierFlags] & NSNumericPadKeyMask) {
    if ( [keys length] == 1 ) {
      if ( keyChar == NSLeftArrowFunctionKey ) {
        self.touchInput->controllers[0]->dpad |= CONTROLLER_DPAD_LEFT;
        return;
      }
      if ( keyChar == NSRightArrowFunctionKey ) {
        self.touchInput->controllers[0]->dpad |= CONTROLLER_DPAD_RIGHT;
        return;
      }
      if ( keyChar == NSUpArrowFunctionKey ) {
        self.touchInput->controllers[0]->dpad |= CONTROLLER_DPAD_UP;
        return;
      }
      if ( keyChar == NSDownArrowFunctionKey ) {
        self.touchInput->controllers[0]->dpad |= CONTROLLER_DPAD_DOWN;
        return;
      }
    }
  } else {
    if (keyChar == ' ') {
      self.touchInput->controllers[0]->jump = 1;
      return;
    }
  }
  [super keyDown:theEvent];
}

- (void)keyUp:(NSEvent *)theEvent {
  // Arrow keys are associated with the numeric keypad
  NSString *keys = [theEvent charactersIgnoringModifiers];
  unichar keyChar = 0;
  if ( [keys length] == 0 ) return;
  keyChar = [keys characterAtIndex:0];
  if ([theEvent modifierFlags] & NSNumericPadKeyMask) {
    if ( [keys length] == 1 ) {
      if ( keyChar == NSLeftArrowFunctionKey ) {
        self.touchInput->controllers[0]->dpad &= ~(CONTROLLER_DPAD_LEFT);
        return;
      }
      if ( keyChar == NSRightArrowFunctionKey ) {
        self.touchInput->controllers[0]->dpad &= ~(CONTROLLER_DPAD_RIGHT);
        return;
      }
      if ( keyChar == NSUpArrowFunctionKey ) {
        self.touchInput->controllers[0]->dpad &= ~(CONTROLLER_DPAD_UP);
        return;
      }
      if ( keyChar == NSDownArrowFunctionKey ) {
        self.touchInput->controllers[0]->dpad &= ~(CONTROLLER_DPAD_DOWN);
        return;
      }
    }
  } else {
    if (keyChar == ' ') {
      self.touchInput->controllers[0]->jump = 0;
      return;
    }
  }
  [super keyUp:theEvent];
}

@end
