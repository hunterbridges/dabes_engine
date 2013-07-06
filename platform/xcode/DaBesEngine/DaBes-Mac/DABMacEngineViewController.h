#import <Cocoa/Cocoa.h>
#import <GLKit/GLKit.h>
#import <DaBes-Mac/DaBes-Mac.h>

extern NSString *kFrameEndNotification;
extern NSString *kNewSceneNotification;
extern NSString *kEntitySelectedNotification;
extern NSString *kEngineReadyForScriptNotification;
extern NSString *kEngineHasScriptErrorNotification;
extern NSString *kEngineHasScriptPanicNotification;

@interface DABMacEngineViewController : NSViewController

@property (nonatomic, assign) Engine *engine;
@property (nonatomic, assign) Scene *scene;
@property (nonatomic, assign) Input *touchInput;
@property (nonatomic, copy, readonly) NSString *bootScript;
@property (nonatomic, copy, readonly) NSString *projectPath;

- (void)restartCurrentScene;
- (void)initEngine;
- (void)reboot;
- (void)tearDown;
  
@end
