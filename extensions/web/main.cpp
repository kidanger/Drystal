#define LUA_API extern

#include <lua.hpp>

int is_web(lua_State* L)
{
#ifdef EMSCRIPTEN
	lua_pushboolean(L, 1);
#else
	lua_pushboolean(L, 0);
#endif
	return 1;
}

int run_js(lua_State* L)
{
	return 0;
}

static const luaL_Reg lib[] =
{
	{"is_web", is_web},
	{"run_js", run_js},
	{NULL, NULL}
};

LUA_API "C" int luaopen_web(lua_State *L)
{
	lua_pushglobaltable(L);

	luaL_setfuncs(L, lib, 0);

	lua_pushliteral(L, LUA_VERSION);
	lua_setfield(L, -2, "_VERSION");
	return 1;
}

