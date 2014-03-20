#include "lua_init.h"

#include <math.h>
lua_State *global_lua = NULL;


#include "../../code/client/client.h"

int LUA_callfunction(lua_State *L, char *functionname, char *params, ...)
{
	va_list args;
	int len, i, errors=0;

	if ( ! L)
		return 0;

	lua_getglobal(global_lua, functionname);

	if (lua_isnil(L, -1)) {
		Com_Printf("[LUA][ENGINE][WARNING] function \"%s\" not found! params=\"%s\"\n", functionname, params);
		return 0;
	}

	len = strlen(params);
	va_start(args, params);
	for (i=0; i<len; i++) {
		switch (params[i]) {
			case 'i': {
				int tmp_int = va_arg(args, int);
				lua_pushinteger(L, tmp_int);
				break;
			}
			default:
				errors++;
				Com_Printf("[WARNING] LUA_callfunction errors=%d params[%d]=%c not implemented!\n", errors, i, params[i]);
		}
	}
	va_end(args);

	lua_call(global_lua, len, 0); // args, 0 rets

	return errors == 0; // success if no errors
}

int traceback(lua_State *L)
{
  if (!lua_isstring(L, 1)) { /* Non-string error object? Try metamethod. */
    if (lua_isnoneornil(L, 1) ||
	!luaL_callmeta(L, 1, "__tostring") ||
	!lua_isstring(L, -1))
      return 1;  /* Return non-string error object. */
    lua_remove(L, 1);  /* Replace object by result of __tostring metamethod. */
  }
  luaL_traceback(L, L, lua_tostring(L, 1), 1);
  return 1;
}

int docall(lua_State *L, int narg, int clear)
{
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, traceback);  /* push traceback function */
  lua_insert(L, base);  /* put it under chunk and args */
#if !LJ_TARGET_CONSOLE && 0
  signal(SIGINT, laction);
#endif
  status = lua_pcall(L, narg, (clear ? 0 : LUA_MULTRET), base);
#if !LJ_TARGET_CONSOLE && 0
  signal(SIGINT, SIG_DFL);
#endif
  lua_remove(L, base);  /* remove traceback function */
  /* force a complete garbage collection in case of errors */
  if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
  return status;
}

void l_message(const char *pname, const char *msg)
{
	if (pname) Com_Printf("%s: ", pname);
	Com_Printf("%s\n", msg);
}

int report(lua_State *L, int status)
{
  if (status && !lua_isnil(L, -1)) {
    const char *msg = lua_tostring(L, -1);
    if (msg == NULL) msg = "(error object is not a string)";
    l_message("", msg);
    lua_pop(L, 1);
  }
  return status;
}

int dofile(lua_State *L, const char *name)
{
  int status = luaL_loadfile(L, name) || docall(L, 0, 1);
  return report(L, status);
}

int dostring(lua_State *L, const char *s, const char *name)
{
  int status = luaL_loadbuffer(L, s, strlen(s), name) || docall(L, 0, 1);
  return report(L, status);
}

int dolibrary(lua_State *L, const char *name)
{
  lua_getglobal(L, "require");
  lua_pushstring(L, name);
  return report(L, docall(L, 1, 1));
}

int l_sin (lua_State *L) {
	double d = luaL_checknumber(L, 1);
	lua_pushnumber(L, sin(d * (M_PI/180)));
	return 1;  /* number of results */
}

int lua_Cvar_Set (lua_State *L) {	
	char *var_name = (char *)luaL_checkstring(L, 1);
	char *value = (char *)luaL_checkstring(L, 2);
	//Cvar_Set(var_name, value);
}
int lua_Com_Printf(lua_State *L) {	
	char *text = (char *)luaL_checkstring(L, 1);
	Com_Printf("%s", text);
	return 0;
}
// need to export it to read fs_game in include(), which would have circle dependency on include("lua\\codscript")
int lua_Cvar_VariableString(lua_State *L) {
	char *name = (char *)luaL_checkstring(L, 1);
	lua_pushstring(L, Cvar_VariableString(name));
	return 1;
}
void LUA_init()
{
	int ret;

	global_lua = lua_open();
	
	lua_pushcfunction(global_lua, l_sin);
	lua_setglobal(global_lua, "mysin");
	
	lua_pushcfunction(global_lua, lua_Cvar_Set);
	lua_setglobal(global_lua, "Cvar_Set");
	
	lua_pushcfunction(global_lua, lua_Com_Printf);
	lua_setglobal(global_lua, "Com_Printf");

	lua_pushcfunction(global_lua, lua_Cvar_VariableString);
	lua_setglobal(global_lua, "Cvar_VariableString");

	// we have 3 lua-engines, so give the scripts some orientation
	lua_pushinteger(global_lua, 0);
	lua_setglobal(global_lua, "CLIENT"); // only cgame.dll
	lua_pushinteger(global_lua, 0);
	lua_setglobal(global_lua, "SERVER"); // only game.dll
	lua_pushinteger(global_lua, 1);
	lua_setglobal(global_lua, "ENGINE"); // only game.exe

	lua_gc(global_lua, LUA_GCSTOP, 0);  /* stop collector during initialization */
	luaL_openlibs(global_lua);  /* open libraries */
	lua_gc(global_lua, LUA_GCRESTART, -1);

	ret = dofile(global_lua, va("%s\\lua\\ENGINE\\main.lua", Cvar_VariableString("fs_game")));
	
	Com_Printf("[ENGINE] LUA_init global_lua=%.8p dofile=%s fs_game=%s\n", global_lua, (!ret)?"success":"fail", Cvar_VariableString("fs_game"));
	
	if (ret) { // 1 means error
		lua_close(global_lua);
		global_lua = NULL;
	}
}