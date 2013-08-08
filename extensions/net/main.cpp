#define LUA_API extern

#include <lua.hpp>

#include "engine.hpp"

#include "network.hpp"

Network net;


static int mlua_connect(lua_State* L)
{
	const char* host = lua_tostring(L, -2);
	int port = lua_tointeger(L, -1);
	bool ok = net.connect(host, port);
	lua_pushnumber(L, ok);
	return 1;
}
static int mlua_send(lua_State* L)
{
	const char* message = lua_tostring(L, -1);
	net.send(message, strlen(message));
	return 0;
}
static int mlua_disconnect(lua_State*)
{
	net.disconnect();
	return 0;
}

static const luaL_Reg lib[] =
{
	{"connect", mlua_connect},
	{"send", mlua_send},
	{"disconnect", mlua_disconnect},
	{NULL, NULL}
};

LUA_API "C" int luaopen_net(lua_State *L)
{
	luaL_newlibtable(L, lib);
	luaL_setfuncs(L, lib, 0);
	return 1;
}


