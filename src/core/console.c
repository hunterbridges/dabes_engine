#include "console.h"
#include "engine.h"

Console *Console_create(ConsoleProto proto) {
  Console *console = calloc(1, sizeof(Console));
  check(console != NULL, "Couldn't create Console");
  
  console->proto = proto;
  
  return console;
error:
  return NULL;
}

void Console_destroy(struct Console *console) {
  check(console != NULL, "No console to destroy");
  free(console);
  return;
error:
  return;
}

////////////////////////////////////////////////////////////////////////////////

void STDIOConsole_log(Console *console, char *str, char *level) {
  printf("%s: %s\n", level, str);
}

ConsoleProto STDIOConsoleProto = {
  .log = STDIOConsole_log
};
