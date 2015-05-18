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
#include <assert.h>
#include <lua.h>
#include <lauxlib.h>

#include "display.h"
#include "buffer_bind.h"
#include "log.h"

log_category("buffer");

IMPLEMENT_PUSHPOP(Buffer, buffer)

int mlua_new_buffer(lua_State* L)
{
	assert(L);

	Buffer* buffer;
	if (lua_gettop(L) == 1) {
		lua_Integer size = luaL_checkinteger(L, 1);
		assert_lua_error(L, size > 0, "size should be positive");
		buffer = display_new_buffer(size);
	} else {
		buffer = display_new_auto_buffer(); // let Display choose a size
	}
	assert(buffer);
	push_buffer(L, buffer);
	return 1;
}

int mlua_use_buffer(lua_State* L)
{
	assert(L);

	Buffer* buffer = pop_buffer(L, 1);
	assert_lua_error(L, !buffer_was_freed(buffer), "cannot use() a freed buffer");
	display_use_buffer(buffer);
	return 0;
}

int mlua_use_default_buffer(_unused_ lua_State* L)
{
	display_use_default_buffer();
	return 0;
}

int mlua_draw_buffer(lua_State* L)
{
	assert(L);

	Buffer* buffer = pop_buffer(L, 1);
	lua_Number dx = 0, dy = 0;
	if (lua_gettop(L) >= 2)
		dx = luaL_checknumber(L, 2);
	if (lua_gettop(L) >= 3)
		dy = luaL_checknumber(L, 3);
	display_draw_buffer(buffer, dx, dy);
	return 0;
}

int mlua_reset_buffer(lua_State* L)
{
	assert(L);

	Buffer* buffer = pop_buffer(L, 1);
	assert_lua_error(L, !buffer_was_freed(buffer), "cannot reset() a freed buffer");
	buffer_reset(buffer);
	return 0;
}

int mlua_upload_and_free_buffer(lua_State* L)
{
	assert(L);

	Buffer* buffer = pop_buffer(L, 1);
	assert_lua_error(L, !buffer_was_freed(buffer), "cannot upload_and_free() a freed buffer");
	buffer_upload_and_free(buffer);
	display_use_default_buffer();
	return 0;
}

int mlua_free_buffer(lua_State* L)
{
	assert(L);

	log_debug("");
	Buffer* buffer = pop_buffer(L, 1);
	display_free_buffer(buffer);
	return 0;
}

