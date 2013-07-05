//
//  SDKInspectoveView.m
//  DaBesEngine
//
//  Created by Hunter Bridges on 6/19/13.
//  Copyright (c) 2013 The Telemetry Group. All rights reserved.
//

#import <DaBes-Mac/DaBes-Mac.h>
#import <Cocoa/Cocoa.h>
#import "SDKMacAppDelegate.h"
#import "SDKInspectorView.h"
#import "SDKPanner.h"

NSString *kControlChangedNotification = @"kControlChangedNotification";

@interface SDKInspectorView ()

@property (nonatomic, strong) IBOutlet NSSlider *cameraZoomSlider;
@property (nonatomic, strong) IBOutlet NSTextField *cameraZoomLabel;
@property (nonatomic, strong) IBOutlet NSButton *cameraResetZoomButton;
@property (nonatomic, strong) IBOutlet NSSlider *cameraRotateSlider;
@property (nonatomic, strong) IBOutlet NSTextField *cameraRotateLabel;
@property (nonatomic, strong) IBOutlet NSButton *cameraResetRotateButton;
@property (nonatomic, strong) IBOutlet SDKPanner *cameraFocalPanner;
@property (nonatomic, strong) IBOutlet NSTextField *cameraFocalLabel;
@property (nonatomic, strong) IBOutlet SDKPanner *cameraOffsetPanner;
@property (nonatomic, strong) IBOutlet NSTextField *cameraOffsetLabel;
@property (nonatomic, strong) IBOutlet NSButton *cameraResetOffsetButton;
@property (nonatomic, strong) IBOutlet NSButton *cameraSnapSceneCheckbox;
@property (nonatomic, strong) IBOutlet NSButton *cameraDebugCheckbox;
@property (nonatomic, strong) IBOutlet NSButton *cameraTrackEntityButton;
@property (nonatomic, assign) BOOL cameraEditingTracking;

@property (nonatomic, strong) NSMapTable *updateMap;
@property (nonatomic, strong) NSMapTable *usingControls;

@property (nonatomic, strong) IBOutlet NSWindow *cameraTrackEntityTip;

@end

@implementation SDKInspectorView

- (id)initWithCoder:(NSCoder *)aDecoder {
  self = [super initWithCoder:aDecoder];
  if (self) {
    self.usingControls =
        [[NSMapTable alloc] initWithKeyOptions:NSMapTableWeakMemory
                                  valueOptions:NSMapTableStrongMemory
                                      capacity:50];
  }
  return self;
}

- (void)dealloc {
  [self.engineVC removeObserver:self forKeyPath:@"scene"];
  [[NSNotificationCenter defaultCenter] removeObserver:self];
}

- (void)awakeFromNib {
  [super awakeFromNib];
  [self constructUpdateMap];
  [[NSNotificationCenter defaultCenter]
      addObserver:self
      selector:@selector(handleFrameEndNotification:)
      name:kFrameEndNotification
      object:nil];
  [[NSNotificationCenter defaultCenter]
      addObserver:self
      selector:@selector(handleNewSceneNotification:)
      name:kNewSceneNotification
      object:nil];
  [[NSNotificationCenter defaultCenter]
      addObserver:self
      selector:@selector(handleEntitySelectedNotification:)
      name:kEntitySelectedNotification
      object:nil];
}

- (void)setEngineVC:(DABMacEngineViewController *)engineVC {
  if (_engineVC) {
    [_engineVC removeObserver:self forKeyPath:@"scene"];
  }
  _engineVC = engineVC;
  [_engineVC addObserver:self
              forKeyPath:@"scene"
                 options:NSKeyValueObservingOptionNew
                 context:NULL];
  [self updateAllLabels:YES];
}

- (void)observeValueForKeyPath:(NSString *)keyPath
                      ofObject:(id)object
                        change:(NSDictionary *)change
                       context:(void *)context {
  if (object == self.engineVC && [keyPath isEqualToString:@"scene"]) {
    [self updateAllLabels:YES];
    return;
  }
  [super observeValueForKeyPath:keyPath
                       ofObject:object
                         change:change
                        context:context];
}

#pragma mark - Private

- (void)handleFrameEndNotification:(NSNotification *)notif {
  // TODO: Hmm... maybe should not force every frame
  [self updateAllLabels:YES];
}

- (void)handleNewSceneNotification:(NSNotification *)notif {
  [self updateAllLabels:YES];
}

- (void)handleEntitySelectedNotification:(NSNotification *)notif {
  if (self.engineVC.scene->selection_mode == kSceneSelectingForCamera) {
    int i = 0;
    if (![notif.userInfo[@"commandKey"] boolValue]) {
      Entity *track[self.engineVC.scene->selected_entities->count];
      LIST_FOREACH(self.engineVC.scene->selected_entities, first, next, current) {
        track[i] = current->value;
        i++;
      }
      Camera_track_entities(self.engineVC.scene->camera, i, track);
      Scene_set_selection_mode(self.engineVC.scene, kSceneNotSelecting);
      self.cameraEditingTracking = NO;
      [self.cameraTrackEntityTip close];
    }
  }
}

- (void)constructUpdateMap {
  self.updateMap =
      [[NSMapTable alloc] initWithKeyOptions:NSMapTableWeakMemory
                                valueOptions:NSMapTableStrongMemory
                                    capacity:50];
  [self.updateMap setObject:NSStringFromSelector(@selector(updateCameraZoomLabel))
                     forKey:(id)self.cameraZoomSlider];
  [self.updateMap setObject:NSStringFromSelector(@selector(updateCameraRotateLabel))
                     forKey:(id)self.cameraRotateSlider];
  [self.updateMap setObject:NSStringFromSelector(@selector(updateCameraFocalLabel))
                     forKey:(id)self.cameraFocalPanner];
  [self.updateMap setObject:NSStringFromSelector(@selector(updateCameraOffsetLabel))
                     forKey:(id)self.cameraOffsetPanner];
}

- (void)updateAllLabels:(BOOL)force {
  if (!self.engineVC.engine) return;
  for (id control in self.updateMap) {
    SEL updateSelector = NSSelectorFromString([self.updateMap objectForKey:control]);
    BOOL updating = [[self.usingControls objectForKey:control] boolValue];
    if (updating || force) {
      if ([self respondsToSelector:updateSelector]) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Warc-performSelector-leaks"
        [self performSelector:updateSelector withObject:nil];
#pragma clang diagnostic pop
      }
    }
  }
  
  if (!self.engineVC.scene) return;
  self.cameraSnapSceneCheckbox.intValue =
      self.engineVC.scene->camera->snap_to_scene;
  self.cameraDebugCheckbox.intValue = self.engineVC.scene->debug_camera;
  BOOL hasEntities = (self.engineVC.scene->camera->num_entities > 0);
  self.cameraTrackEntityButton.state =
      hasEntities || self.cameraEditingTracking ? NSOnState : NSOffState;
  [self.cameraFocalPanner setEnabled:!hasEntities];
}

- (void)updateInt:(int *)outval withSlider:(NSSlider *)slider {
  NSEvent *event = [[NSApplication sharedApplication] currentEvent];
  BOOL startingDrag = event.type == NSLeftMouseDown;
  BOOL endingDrag = event.type == NSLeftMouseUp;
  BOOL dragging = event.type == NSLeftMouseDragged;
  
  if (startingDrag) {
    [self.usingControls setObject:@(YES) forKey:slider];
  }
  
  if (startingDrag || dragging) {
    slider.intValue = slider.intValue;
  }
  
  if (endingDrag) {
    [self.usingControls setObject:@(NO) forKey:slider];
    slider.intValue = 0;
  }
  
  *outval = slider.intValue;
  [self updateAllLabels:NO];
}

- (void)updatePoint:(VPoint *)point withPanner:(SDKPanner *)panner {
  VPoint targetPoint = {panner.cgPointValue.x, panner.cgPointValue.y};
  BOOL isZero = CGPointEqualToPoint(panner.cgPointValue, CGPointZero);
  
  if (isZero) {
    [self.usingControls setObject:@(NO) forKey:panner];
  } else {
    [self.usingControls setObject:@(YES) forKey:panner];
  }
  
  *point = targetPoint;
  [self updateAllLabels:NO];
}

#pragma mark - Label Updaters

- (void)updateCameraZoomLabel {
  Scene *scene = self.engineVC.scene;
  self.cameraZoomLabel.stringValue =
      [NSString stringWithFormat:@"%.2f", scene ? scene->camera->scale : 0];
  [self.cameraZoomLabel setNeedsDisplay];
}

- (void)updateCameraRotateLabel {
  Scene *scene = self.engineVC.scene;
  float degs = scene ? scene->camera->rotation_radians * 180 / M_PI : 0;
  self.cameraRotateLabel.stringValue =
      [NSString stringWithFormat:@"%d°", (int)roundf(degs)];
  [self.cameraRotateLabel setNeedsDisplay];
}

- (void)updateCameraFocalLabel {
  Scene *scene = self.engineVC.scene;
  VPoint focal = scene ? scene->camera->focal : VPointZero;
  self.cameraFocalLabel.stringValue =
      [NSString stringWithFormat:@"%d, %d", (int)focal.x, (int)focal.y];
}

- (void)updateCameraOffsetLabel {
  Scene *scene = self.engineVC.scene;
  VPoint offset = scene ? scene->camera->translation : VPointZero;
  self.cameraOffsetLabel.stringValue =
      [NSString stringWithFormat:@"%d, %d", (int)offset.x, (int)offset.y];
}

#pragma mark - IBActions

- (IBAction)zoomSliderChanged:(id)sender {
  [self updateInt:&(self.touchInput->cam_zoom) withSlider:sender];
}

- (IBAction)resetZoomClicked:(id)sender {
  if (!self.engineVC.scene) return;
  self.engineVC.scene->camera->scale = 1.0;
}

- (IBAction)rotSliderChanged:(id)sender {
  [self updateInt:&(self.touchInput->cam_rotate) withSlider:sender];
}

- (IBAction)resetRotateClicked:(id)sender {
  if (!self.engineVC.scene) return;
  self.engineVC.scene->camera->rotation_radians = 0.0;
}

- (IBAction)focalPannerChanged:(id)sender {
  [self updatePoint:&(self.touchInput->cam_focal_pan) withPanner:sender];
}

- (IBAction)offetPannerChanged:(id)sender {
  [self updatePoint:&(self.touchInput->cam_translate_pan) withPanner:sender];
}

- (IBAction)resetOffsetClicked:(id)sender {
  if (!self.engineVC.scene) return;
  self.engineVC.scene->camera->translation = VPointZero;
}

- (IBAction)snapSceneChanged:(id)sender {
  NSButton *button = sender;
  
  if (!self.engineVC.scene) return;
  self.engineVC.scene->camera->snap_to_scene = button.intValue;
}

- (IBAction)debugOverlayChanged:(id)sender {
  NSButton *button = sender;
  
  if (!self.engineVC.scene) return;
  self.engineVC.scene->debug_camera = button.intValue;
}

- (IBAction)trackEntitiesClicked:(id)sender {
  NSButton *button = sender;
  if (!self.engineVC.scene) return;
  if (button.state == NSOffState) {
    // Untrack
    Camera_track_entities(self.engineVC.scene->camera, 0, NULL);
    self.cameraEditingTracking = NO;
    [self.cameraTrackEntityTip close];
    Scene_set_selection_mode(self.engineVC.scene, kSceneNotSelecting);
  } else {
    // Track
    NSWindow *mainWindow = [(SDKMacAppDelegate *)[NSApp delegate] engineWindow];
    CGFloat titleHeight = mainWindow.frame.size.height -
        ((NSView *)mainWindow.contentView).frame.size.height;
    NSRect mainWindowRect = mainWindow.frame;
    CGRect cgWinRect = NSRectToCGRect(mainWindowRect);
    
    [self.cameraTrackEntityTip makeKeyAndOrderFront:nil];
    NSRect tipRect = self.cameraTrackEntityTip.frame;
    tipRect.origin.x = CGRectGetMaxX(cgWinRect) - CGRectGetWidth(tipRect);
    tipRect.origin.y =
        CGRectGetMaxY(cgWinRect) - CGRectGetHeight(tipRect) - titleHeight;
    [self.cameraTrackEntityTip setFrame:tipRect display:YES];
    self.cameraEditingTracking = YES;
    
    [mainWindow makeKeyAndOrderFront:nil];
    
    Scene_set_selection_mode(self.engineVC.scene, kSceneSelectingForCamera);
  }
}

@end
