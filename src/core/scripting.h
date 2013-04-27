#ifndef __scripting_h
#define __scripting_h
#include <lua/lua.h>
#include <lua/lualib.h>
#include <lua/lauxlib.h>
#include "../prefix.h"

typedef struct Scripting {
    lua_State *L;
} Scripting;

Scripting *Scripting_create(const char *boot_script);
void Scripting_destroy(Scripting *scripting);
int Scripting_test(Scripting *scripting);

#define Scripting_bail(L, MSG) { \
    fprintf(stderr, "%s: %s\n", MSG, lua_tostring(L, -1)); \
    return 0; \
}

extern const char *SCRIPTING_CL_ENTITY_CONFIG;
extern const char *SCRIPTING_CL_PARALLAX;
extern const char *SCRIPTING_CL_PARALLAX_LAYER;

#endif
