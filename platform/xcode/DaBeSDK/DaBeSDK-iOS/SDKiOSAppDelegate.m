#import <DaBes-iOS/DaBes-iOS.h>

#import "SDKiOSAppDelegate.h"

#import "SDKiOSProjectEngineViewController.h"
#import "InterfaceViewController.h"

static const NSString *kOpenMapOptionKey = @"OpenMap";

void Engine_log_iOS(char *fmt, ...) {
  SDKiOSAppDelegate *appDelegete = [[UIApplication sharedApplication] delegate];
  InterfaceViewController *ivc = appDelegete.viewController;
  
  va_list args;
  NSString *nsFmt = [[NSString alloc] initWithCString:fmt
                                             encoding:NSUTF8StringEncoding];
  va_start(args, fmt);
  [ivc log:nsFmt arguments:args];
  va_end(args);
}

@implementation SDKiOSAppDelegate

- (BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
  NSURL *url = [launchOptions valueForKey:UIApplicationLaunchOptionsURLKey];
  NSString *sourceApp =
      [launchOptions valueForKey:
          UIApplicationLaunchOptionsSourceApplicationKey];
    
  if (url) {
    return [self application:application
                     openURL:url
           sourceApplication:sourceApp
                  annotation:nil];
  }
  
  if (sourceApp) {
    return YES;
  }
  
  self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
  self.viewController = [[InterfaceViewController alloc] init];
  self.window.rootViewController = self.viewController;
  [self.window makeKeyAndVisible];
  
  NSString *openMapPath = [launchOptions objectForKey:kOpenMapOptionKey];
  if (openMapPath) {
    [self.viewController injectMapFromPath:openMapPath];
  }
  return YES;
}

- (void)applicationWillResignActive:(UIApplication *)application {
  // Sent when the application is about to move from active to inactive state. This can occur for certain types of temporary interruptions (such as an incoming phone call or SMS message) or when the user quits the application and it begins the transition to the background state.
  // Use this method to pause ongoing tasks, disable timers, and throttle down OpenGL ES frame rates. Games should use this method to pause the game.
}

- (void)applicationDidEnterBackground:(UIApplication *)application {
  [self.viewController didEnterBackground];
}

- (void)applicationWillEnterForeground:(UIApplication *)application {
  // Called as part of the transition from the background to the inactive state; here you can undo many of the changes made on entering the background.
}

- (void)applicationDidBecomeActive:(UIApplication *)application {
  [self.viewController willEnterForeground];
}

- (void)applicationWillTerminate:(UIApplication *)application {
  // Called when the application is about to terminate. Save data if appropriate. See also applicationDidEnterBackground:.
}

- (BOOL)application:(UIApplication *)application
            openURL:(NSURL *)url
  sourceApplication:(NSString *)sourceApplication
         annotation:(id)annotation {
  NSArray *paths =
      NSSearchPathForDirectoriesInDomains(NSDocumentDirectory,
                                          NSUserDomainMask,
                                          YES);
  NSString *documentsDirectory = [paths objectAtIndex:0];
  
  NSString *newFilePath =
      [documentsDirectory stringByAppendingPathComponent:
          [url lastPathComponent]];
  
  if ([[NSFileManager defaultManager] fileExistsAtPath:newFilePath]) {
    [[NSFileManager defaultManager] removeItemAtPath:newFilePath
                                               error:nil];
  }
  [[NSFileManager defaultManager] copyItemAtURL:url
      toURL:[NSURL fileURLWithPath:newFilePath]
      error:nil];
  
  if (!self.window) {
    NSDictionary *dict =
        [NSDictionary dictionaryWithObject:newFilePath
                                    forKey:kOpenMapOptionKey];
    [self application:application didFinishLaunchingWithOptions:dict];
  } else {
    [self.viewController injectMapFromPath:newFilePath];
  }
  
  return YES;
}

@end
