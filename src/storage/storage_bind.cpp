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
#ifndef EMSCRIPTEN
#include <cstdlib>
#endif

#include "lua_util.hpp"
#include "storage_bind.hpp"
#include "storage.hpp"

extern "C" {
	extern int json_encode(lua_State* L);
	extern int json_decode(lua_State* L);
}

int mlua_store(lua_State* L)
{
	assert(L);

	const char* key = luaL_checkstring(L, 1);

	lua_pushcfunction(L, json_encode);
	lua_pushvalue(L, 2);

	CALL(1, 1); // table in param, returns json

	const char* value = luaL_checkstring(L, -1);

	store(key, value);
	return 0;
}

int mlua_fetch(lua_State* L)
{
	assert(L);

	const char* key = luaL_checkstring(L, 1);
	char* value = fetch(key);

	if (!value || !value[0]) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushcfunction(L, json_decode);
	lua_pushstring(L, value);
	CALL(1, 1);

#ifndef EMSCRIPTEN
	free(value);
#endif
	// table is returned by json_decode
	return 1;
}

