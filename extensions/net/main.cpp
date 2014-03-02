#include <cstring>

#include <lua.hpp>

#include "engine.hpp"

#include "network.hpp"

Network net;
char reception_buffer[128];

static int mlua_connect(lua_State* L)
{
	const char* host = luaL_checkstring(L, 1);
	int port = luaL_checkint(L, 2);
	ErrorCode error = net.connect(host, port);
	lua_pushnumber(L, error);
	return 1;
}
static int mlua_send(lua_State* L)
{
	size_t size;
	const char* message = luaL_checklstring(L, 1, &size);
	int sent_or_error = net.send(message, size);
	lua_pushnumber(L, sent_or_error);
	return 1;
}
static int mlua_receive(lua_State* L)
{
	reception_buffer[0] = 0; // assure that even if Network doesn't touch the buffer, it's still valid
	long received_or_error = net.receive(reception_buffer, sizeof(reception_buffer));
	lua_pushnumber(L, received_or_error);
	if (received_or_error > 0) {
		reception_buffer[received_or_error] = 0;
		lua_pushstring(L, reception_buffer);
		return 2;
	}
	return 1;
}
static int mlua_disconnect(lua_State* L)
{
	ErrorCode error = net.disconnect();
	lua_pushnumber(L, error);
	return 1;
}

static const luaL_Reg lib[] =
{
	{"connect", mlua_connect},
	{"send", mlua_send},
	{"receive", mlua_receive},
	{"disconnect", mlua_disconnect},
	{NULL, NULL}
};

DEFINE_EXTENSION(net)
{
	luaL_newlib(L, lib);

	lua_pushnumber(L, NO_ERROR);
	lua_setfield(L, -2, "NO_ERROR");
	lua_pushnumber(L, UNABLE_TO_OPEN_SOCKET);
	lua_setfield(L, -2, "UNABLE_TO_OPEN_SOCKET");
	lua_pushnumber(L, UNABLE_TO_GET_HOST);
	lua_setfield(L, -2, "UNABLE_TO_GET_HOST");
	lua_pushnumber(L, UNABLE_TO_CONNECT);
	lua_setfield(L, -2, "UNABLE_TO_CONNECT");
	lua_pushnumber(L, NOT_CONNECTED);
	lua_setfield(L, -2, "NOT_CONNECTED");
	lua_pushnumber(L, CONNECTION_LOST);
	lua_setfield(L, -2, "CONNECTION_LOST");
	lua_pushnumber(L, ALREADY_DISCONNECTED);
	lua_setfield(L, -2, "ALREADY_DISCONNECTED");
	lua_pushnumber(L, CANNOT_CLOSE_SOCKET);
	lua_setfield(L, -2, "CANNOT_CLOSE_SOCKET");

	return 1;
}

