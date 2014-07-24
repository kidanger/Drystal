/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option); any later version.
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

#include "engine.hpp"
#include "buffer_bind.hpp"
#include "lua_functions.hpp"

DECLARE_PUSHPOP(Buffer, buffer)

int mlua_new_buffer(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Buffer* buffer;
	if (lua_gettop(L) == 1) {
		lua_Number size = luaL_checknumber(L, 1);
		buffer = engine.display.new_buffer(size);
	} else {
		buffer = engine.display.new_buffer(); // let Display choose a size
	}
	if (buffer) {
		push_buffer(L, buffer);
		return 1;
	}
	return 0; // returns nil
}

int mlua_use_buffer(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	if (lua_gettop(L) == 0) { // use defaut buffer
		engine.display.use_buffer(NULL);
	} else {
		Buffer* buffer = pop_buffer(L, 1);
		engine.display.use_buffer(buffer);
	}
	return 0;
}

int mlua_draw_buffer(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Buffer* buffer = pop_buffer(L, 1);
	lua_Number dx = 0, dy = 0;
	if (lua_gettop(L) >= 2)
		dx = luaL_checknumber(L, 2);
	if (lua_gettop(L) >= 3)
		dy = luaL_checknumber(L, 3);
	engine.display.draw_buffer(buffer, dx, dy);
	return 0;
}

int mlua_reset_buffer(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Buffer* buffer = pop_buffer(L, 1);
	engine.display.reset_buffer(buffer);
	return 0;
}

int mlua_upload_and_free_buffer(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Buffer* buffer = pop_buffer(L, 1);
	engine.display.upload_and_free_buffer(buffer);
	return 0;
}

int mlua_free_buffer(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Buffer* buffer = pop_buffer(L, 1);
	engine.display.free_buffer(buffer);
	return 0;
}

