#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>
#import <DaBes-iOS/DaBes-iOS.h>

extern NSString *kFrameEndNotification;
extern NSString *kEngineReadyForScriptNotification;
extern NSString *kEntitySelectedNotification;

@interface DABiOSEngineViewController : GLKViewController <UIGestureRecognizerDelegate,
                                              GLKViewDelegate>

+ (DABiOSEngineViewController *)sharedInstance;
- (id)initWithTouchInput:(Input *)input;
- (void)didEnterBackground;
- (void)willEnterForeground;
- (void)injectMapFromPath:(NSString *)newFilePath;
- (void)restartScene;
- (void)fullUpdate;
- (void)reboot;
  
@property (nonatomic, readonly) Engine *engine;
@property (nonatomic, readonly) Scene *scene;
@property (nonatomic, readonly) GLKView *glkView;
@property (nonatomic, copy, readonly) NSString *bootScript;
@property (nonatomic, copy, readonly) NSString *projectPath;

@end
