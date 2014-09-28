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
#include <lua.hpp>

#include "lua_util.hpp"
#include "socket.hpp"
#include "server.hpp"
#include "server_bind.hpp"
#include "socket_bind.hpp"

static Server server;

int mlua_listen(lua_State* L)
{
	assert(L);

	int port = luaL_checkint(L, 1);

	assert_lua_error(L, port >= 0 && port < 65536, "listen: the port number must be >= 0 and < 65536");

	server.listen(port);
	return 0;
}

int mlua_accept(lua_State* L)
{
	assert(L);

	lua_Number timeout = luaL_optnumber(L, 2, 0);
	lua_Number timepassed = 0;
	int max = luaL_optint(L, 3, -1);
	int i = 0;
	while ((max == -1 || i < max) && timepassed <= timeout) {
		lua_Number time;
		Socket* sock = server.accept(timeout - timepassed, &time);
		timepassed += time;

		if (!sock)
			break;

		lua_pushvalue(L, 1); // push the callback

		lua_newtable(L);
		int ref = luaL_ref(L, LUA_REGISTRYINDEX);
		sock->setTable(ref);
		push_socket(L, sock);

		CALL(1, 0);

		i += 1;
	}
	lua_pushnumber(L, timepassed);
	return 1;
}

