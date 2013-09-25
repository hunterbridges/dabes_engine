#include <lcthw/bstrlib.h>
#include "console_bindings.h"

const char *luab_Console_lib = "console";


void concat_log_message(lua_State *L, bstring *message) {
  
  int c = lua_gettop(L);
  
  bstring space = bfromcstr(" ");
  int i = 0;
  for (i = 0; i < c; i++) {
    const char *str = lua_tostring(L, 1);
    bstring bstr = bfromcstr(str);
    if (i > 0) {
      bconcat(message, space);
    }
    bconcat(message, bstr);
    bdestroy(bstr);
    lua_remove(L, 1);
  }
  bdestroy(space);
}


void log_console_message(lua_State *L, char *level) {
  Engine *engine = luaL_get_engine(L);
  
  // Manipulate bstr with var args
  
  bstring totalstr = bfromcstr("");
  concat_log_message(L, totalstr);
  
  char *bcstr = bstr2cstr(totalstr, ' ');
  engine->console->_(log)(engine->console, bcstr, level);
  bcstrfree(bcstr);
  
  bdestroy(totalstr);
}

int luab_Console_log(lua_State *L) {
  log_console_message(L, "LOG");
  
  return 0;
}

int luab_Console_warn(lua_State *L) {
  log_console_message(L, "WARN");
  
  return 0;
}

int luab_Console_error(lua_State *L) {
  log_console_message(L, "ERROR");
  
  return 0;
}

static const struct luaL_Reg luab_Console_funcs[] = {
  {"log", luab_Console_log},
  {"warn", luab_Console_warn},
  {"error", luab_Console_error},
  {NULL, NULL}
};

int luaopen_dabes_console(lua_State *L) {
  luaL_newlib(L, luab_Console_funcs);
  lua_pushvalue(L, -1);
  lua_setglobal(L, luab_Console_lib);
  
  return 1;
}