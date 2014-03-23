#ifndef _LUA_INIT_H_
#define _LUA_INIT_H_

#define luajit_c // ???
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include "luajit.h"

// either it's defined by Visual Studio Project to 1 or define to 0 here
#ifndef LUA_CLIENT
	#define LUA_CLIENT 0
#endif
#ifndef LUA_SERVER
	#define LUA_SERVER 0
#endif
#ifndef LUA_ENGINE
	#define LUA_ENGINE 0
#endif

#if LUA_CLIENT
	#define LUA_BINARY "CLIENT"
#endif
#if LUA_SERVER
	#define LUA_BINARY "SERVER"
#endif
#if LUA_ENGINE
	#define LUA_BINARY "ENGINE"
#endif

extern lua_State *global_lua;

int LUA_callfunction(lua_State *L, char *functionname, char *params, ...);

int traceback(lua_State *L);
int docall(lua_State *L, int narg, int clear);
void l_message(const char *pname, const char *msg);
int report(lua_State *L, int status);
int dofile(lua_State *L, const char *name);
int dostring(lua_State *L, const char *s, const char *name);
int dolibrary(lua_State *L, const char *name);
int l_sin (lua_State *L);
int lua_Cvar_Set (lua_State *L);
int lua_Com_Printf(lua_State *L);
void LUA_init();

#endif