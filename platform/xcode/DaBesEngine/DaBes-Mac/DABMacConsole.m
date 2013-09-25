#import "DABMacConsole.h"

NSString *kDABMacConsoleLogNotification = @"kDABMacConsoleLogNotification";

void DABMacConsole_log(Console *log, char *str, char *level) {
  NSString *msg =
      [NSString stringWithCString:str encoding:NSUTF8StringEncoding];
  NSString *lvl = [NSString stringWithCString:level encoding:NSUTF8StringEncoding];
  [[NSNotificationCenter defaultCenter]
      postNotificationName:kDABMacConsoleLogNotification
      object:nil
      userInfo:@{@"message": msg, @"level": lvl}];
  printf("[%s] %s\n", level, str);
}

ConsoleProto DABMacConsoleProto = {
  .log = DABMacConsole_log
};