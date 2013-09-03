#import <Carbon/Carbon.h>
#import <DaBes-Mac/DaBes-Mac.h>
#import "DABMacEngineViewController.h"
#import "DABMacEngineView.h"

@interface DABMacEngineView ()

@property (nonatomic, assign) Input *touchInput;
@property (nonatomic, assign) BOOL wasAcceptingMouseEvents;
@property (nonatomic, assign) NSTrackingRectTag trackingRect;

@end

@implementation DABMacEngineView

static unsigned short int keysDown[128];

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

- (void)setScene:(Scene *)scene {
  _scene = scene;
  [self resizeGraphics];
}

- (void)drawRect:(NSRect)dirtyRect
{
  // Drawing code here.
  [self.openGLContext makeCurrentContext];
  if (!self.engine) return;
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  Scene *scene = self.scene;
  if (!scene) return;
  Scene_render(scene, self.engine);
  [self.openGLContext flushBuffer];
}

- (void)keyDown:(NSEvent *)theEvent {
  // Arrow keys are associated with the numeric keypad
  unsigned keyCode = [theEvent keyCode];
  if (keyCode < 128) keysDown[keyCode] = YES;
  
  if (keyCode == kVK_LeftArrow)
    self.touchInput->controllers[0]->dpad |= CONTROLLER_DPAD_LEFT;
  if (keyCode == kVK_RightArrow)
    self.touchInput->controllers[0]->dpad |= CONTROLLER_DPAD_RIGHT;
  if (keyCode == kVK_UpArrow)
    self.touchInput->controllers[0]->dpad |= CONTROLLER_DPAD_UP;
  if (keyCode == kVK_DownArrow)
    self.touchInput->controllers[0]->dpad |= CONTROLLER_DPAD_DOWN;
  if (keyCode == kVK_Space)
    self.touchInput->controllers[0]->jump = 1;
}

- (void)keyUp:(NSEvent *)theEvent {
  // Arrow keys are associated with the numeric keypad
  unsigned keyCode = [theEvent keyCode];
  if (keyCode < 128) keysDown[keyCode] = NO;
  
  if (keyCode == kVK_LeftArrow)
    self.touchInput->controllers[0]->dpad &= ~(CONTROLLER_DPAD_LEFT);
  if (keyCode == kVK_RightArrow)
    self.touchInput->controllers[0]->dpad &= ~(CONTROLLER_DPAD_RIGHT);
  if (keyCode == kVK_UpArrow)
    self.touchInput->controllers[0]->dpad &= ~(CONTROLLER_DPAD_UP);
  if (keyCode == kVK_DownArrow)
    self.touchInput->controllers[0]->dpad &= ~(CONTROLLER_DPAD_DOWN);
  if (keyCode == kVK_Space)
    self.touchInput->controllers[0]->jump = 0;
}

- (void)resizeGraphics {
  GfxSize screen_size = {self.bounds.size.width, self.bounds.size.height};
  self.engine->graphics->screen_size = screen_size;
  Scene *scene = self.scene;
  if (scene) {
    scene->camera->screen_size = screen_size;
  }
  glViewport(0, 0, self.bounds.size.width, self.bounds.size.height);
}

- (void)viewWillMoveToSuperview:(NSView *)newSuperview {
  [super viewWillMoveToSuperview:newSuperview];
  if (newSuperview) {
    [self resizeGraphics];
  }
}

- (void)reshape {
  [super reshape];
  [[self openGLContext] update];
  [self resizeGraphics];
  [self setNeedsDisplay:YES];
}
#pragma mark - Mouse Tracking

- (void)resetTrackingRect {
    [self removeTrackingRect:self.trackingRect];
    CGRect tracking = self.bounds;
    self.trackingRect =
        [self addTrackingRect:tracking owner:self userData:NULL assumeInside:YES];
}

- (void)setFrame:(NSRect)frameRect {
    [super setFrame:frameRect];
    [self resetTrackingRect];
}

- (void)setBounds:(NSRect)aRect {
    [super setBounds:aRect];
    [self resetTrackingRect];
}

- (VPoint)screenPointFromMouseEvent:(NSEvent *)theEvent {
    NSPoint mouse = [self.window convertScreenToBase:[NSEvent mouseLocation]];
    NSPoint converted = [self convertPoint:mouse fromView:nil];
    CGPoint cgConv = NSPointToCGPoint(converted);
    VPoint screen_point = {cgConv.x, self.bounds.size.height - cgConv.y};
    return screen_point;
}

- (void)mouseDown:(NSEvent *)theEvent {
    Scene *scene = self.scene;
    if (!scene) return;
    VPoint screen_point = [self screenPointFromMouseEvent:theEvent];
    
    Input *input = self.touchInput;
    input->controllers[0]->touch_state |= CONTROLLER_TOUCH_HOLD_CHANGED;
    input->controllers[0]->touch_state |= CONTROLLER_TOUCH_HOLD;
    input->controllers[0]->touch_pos = screen_point;
    
    int selected = Scene_select_entities_at(scene, screen_point);
    if (selected) {
        BOOL commandKeyPressed = ([theEvent modifierFlags] & NSCommandKeyMask) != 0;
        [[NSNotificationCenter defaultCenter]
            postNotificationName:kEntitySelectedNotification
            object:self
            userInfo:@{@"commandKey": @(commandKeyPressed)}];
    }
}

- (void)mouseUp:(NSEvent *)theEvent {
    Scene *scene = self.scene;
    if (!scene) return;
    VPoint screen_point = [self screenPointFromMouseEvent:theEvent];
    Input *input = self.touchInput;
    input->controllers[0]->touch_state |= CONTROLLER_TOUCH_HOLD_CHANGED;
    input->controllers[0]->touch_state &= ~(CONTROLLER_TOUCH_HOLD);
    input->controllers[0]->touch_pos = screen_point;
}

- (void)mouseMoved:(NSEvent *)theEvent {
    Scene *scene = self.scene;
    if (!scene) return;
    VPoint screen_point = [self screenPointFromMouseEvent:theEvent];
    Input *input = self.touchInput;
    input->controllers[0]->touch_state |= CONTROLLER_TOUCH_MOVED;
    input->controllers[0]->touch_pos = screen_point;
}

- (void)mouseDragged:(NSEvent *)theEvent {
    Scene *scene = self.scene;
    if (!scene) return;
    VPoint screen_point = [self screenPointFromMouseEvent:theEvent];
    Input *input = self.touchInput;
    input->controllers[0]->touch_state |= CONTROLLER_TOUCH_MOVED;
    input->controllers[0]->touch_pos = screen_point;
}

- (void)mouseEntered:(NSEvent *)theEvent {
    self.wasAcceptingMouseEvents = [[self window] acceptsMouseMovedEvents];
    [[self window] setAcceptsMouseMovedEvents:YES];
}

- (void)mouseExited:(NSEvent *)theEvent {
    [[self window] setAcceptsMouseMovedEvents:self.wasAcceptingMouseEvents];
}

@end
