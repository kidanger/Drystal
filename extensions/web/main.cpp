#define LUA_API extern

#include <lua.hpp>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "engine.hpp"

int is_web(lua_State* L)
{
#ifdef EMSCRIPTEN
	lua_pushboolean(L, 1);
#else
	lua_pushboolean(L, 0);
#endif
	return 1;
}

#ifdef EMSCRIPTEN
static void onsuccess(const char* filename)
{
	lua_State* L = get_engine().lua.L;
	lua_getglobal(L, "on_wget_success");
	if (not lua_isfunction(L, -1)) {
		lua_pop(L, 1);
		return;
	}
	lua_pushstring(L, filename);
	if (lua_pcall(L, 1, 0, 0)) {
		luaL_error(L, "error calling on_wget_success: %s", lua_tostring(L, -1));
	}
}

static void onerror(const char* filename)
{
	lua_State* L = get_engine().lua.L;
	lua_getglobal(L, "on_wget_error");
	if (not lua_isfunction(L, -1)) {
		lua_pop(L, 1);
		return;
	}
	lua_pushstring(L, filename);
	if (lua_pcall(L, 1, 0, 0)) {
		luaL_error(L, "error calling on_wget_error: %s", lua_tostring(L, -1));
	}
}

static int wget(lua_State *L)
{
	const char *url = lua_tostring(L, 1);
	const char *filename = lua_tostring(L, 2);
	emscripten_async_wget(url, filename, onsuccess, onerror);
	lua_pushboolean(L, true);
	return 1;
}

#else
static int wget(lua_State *L)
{
	lua_pushboolean(L, false);
	return 1;
}
#endif

int run_js(lua_State* L)
{
#ifdef EMSCRIPTEN
	const char* script = luaL_checkstring(L, 1);
	emscripten_run_script(script);
#endif
	return 0;
}

static const luaL_Reg lib[] =
{
	{"is_web", is_web},
	{"wget", wget},
	{"run_js", run_js},
	{NULL, NULL}
};

LUA_API "C" int luaopen_web(lua_State *L)
{
	luaL_newlibtable(L, lib);
	luaL_setfuncs(L, lib, 0);
	return 1;
}

