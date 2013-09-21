#include "console_bindings.h"

const char *luab_Console_lib = "console";

int luab_Console_log(lua_State *L) {
  Engine *engine = luaL_get_engine(L);
  
  char *str = (char *)lua_tostring(L, 1);
  engine->console->_(log)(engine->console, str);
  
  return 0;
}

static const struct luaL_Reg luab_Console_funcs[] = {
  {"log", luab_Console_log},
  {NULL, NULL}
};

int luaopen_dabes_console(lua_State *L) {
  luaL_newlib(L, luab_Console_funcs);
  lua_pushvalue(L, -1);
  lua_setglobal(L, luab_Console_lib);
  
  return 1;
}