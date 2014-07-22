/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drystal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Drystal.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cassert>
#include <errno.h>
#include <cstring>
#include <lua.hpp>

#include "lua_functions.hpp"
#include "socket.hpp"
#include "socket_bind.hpp"

DECLARE_PUSHPOP(Socket, socket)

int mlua_connect(lua_State* L)
{
	assert(L);

	const char* host = luaL_checkstring(L, 1);
	int port = luaL_checkint(L, 2);
	Socket* socket = Socket::connect(host, port);
	if (socket) {
		lua_newtable(L);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		socket->setTable(ref);
		push_socket(L, socket);
		return 1;
	}
	return 0;
}

int mlua_socket_class_index(lua_State* L)
{
	assert(L);

	Socket* socket = pop_socket(L, 1);
	const char* index = luaL_checkstring(L, 2);
	if (strcmp(index, "address") == 0) {
		lua_pushstring(L, socket->getAddress());
	} else {
		lua_rawgeti(L, LUA_REGISTRYINDEX, socket->getTable());
		lua_getfield(L, -1, index);
		if (lua_isnoneornil(L, -1)) {
			lua_pop(L, 2);
			lua_getmetatable(L, 1);
			lua_getfield(L, -1, index);
		}
	}
	return 1;
}

int mlua_socket_class_newindex(lua_State* L)
{
	assert(L);

	Socket* socket = pop_socket(L, 1);
	lua_rawgeti(L, LUA_REGISTRYINDEX, socket->getTable());
	lua_replace(L, 1);
	lua_rawset(L, 1);
	return 0;
}

int mlua_send_socket(lua_State* L)
{
	assert(L);

	Socket* socket = pop_socket(L, 1);
	size_t size;
	const char* message = luaL_checklstring(L, 2, &size);
	bool error = false;
	socket->send(message, size, &error);
	if (error) {
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	return 0;
}

static char buffer[1024];
int mlua_recv_socket(lua_State* L)
{
	assert(L);

	Socket* socket = pop_socket(L, 1);
	bool error = false;
	int len = socket->receive(buffer, sizeof(buffer), &error);
	if (error) {
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	if (len) {
		lua_pushlstring(L, buffer, len);
		return 1;
	}
	return 0;
}

int mlua_flush_socket(lua_State* L)
{
	assert(L);

	Socket* socket = pop_socket(L, 1);
	bool error = false;
	socket->flush(&error);
	if (error) {
		lua_pushnil(L);
		lua_pushstring(L, strerror(errno));
		return 2;
	}
	return 0;
}

int mlua_disconnect_socket(lua_State* L)
{
	assert(L);

	Socket* socket = pop_socket(L, 1);
	socket->disconnect();
	return 0;
}

int mlua_free_socket(lua_State* L)
{
	assert(L);

	Socket* socket = pop_socket(L, 1);
	luaL_unref(L, LUA_REGISTRYINDEX, socket->getTable());
	delete socket;
	return 0;
}

