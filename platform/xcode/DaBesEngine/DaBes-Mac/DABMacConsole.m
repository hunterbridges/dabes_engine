#import "DABMacConsole.h"

NSString *kDABMacConsoleLogNotification = @"kDABMacConsoleLogNotification";

void DABMacConsole_log(Console *log, char *str) {
  NSString *msg =
      [NSString stringWithCString:str encoding:NSUTF8StringEncoding];
  [[NSNotificationCenter defaultCenter]
      postNotificationName:kDABMacConsoleLogNotification
      object:nil
      userInfo:@{@"message": msg}];
  printf("%s\n", str);
}

ConsoleProto DABMacConsoleProto = {
  .log = DABMacConsole_log
};