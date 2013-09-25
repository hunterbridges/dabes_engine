#ifndef __console_h
#define __console_h

struct Console;
struct Engine;
typedef struct ConsoleProto {
  void (*log)(struct Console *console, char *str, char *level);
} ConsoleProto;

typedef struct Console {
  ConsoleProto proto;
} Console;

Console *Console_create(ConsoleProto proto);
void Console_destroy(struct Console *console);

extern ConsoleProto STDIOConsoleProto;

#endif
