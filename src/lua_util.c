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
#include <assert.h>
#include <lua.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "engine.h"
#include "lua_util.h"
#include "util.h"
#include "livecoding.h"
#include "log.h"

log_category("lua");

int traceback(lua_State *L)
{
	lua_getglobal(L, "debug");
	lua_getfield(L, -1, "newtraceback");
	lua_insert(L, 1);
	lua_pop(L, 1);

	lua_pushinteger(L, 1);
	lua_pushboolean(L, stderr_use_colors());

	lua_call(L, lua_gettop(L) - 1, 1);
	fprintf(stderr, "%s\n", lua_tostring(L, -1));
	return 0;
}

void call_lua_function(lua_State *L, int num_args, int num_ret)
{
	assert(L);
	assert(num_args >= 0);
	assert(num_ret >= 0);

#ifdef EMSCRIPTEN
	lua_call(L, num_args, num_ret);
#else
	/* from lua/src/lua.c */
	int base = lua_gettop(L) - num_args;
	lua_pushcfunction(L, traceback);
	lua_insert(L, base);
	if (lua_pcall(L, num_args, num_ret, base)) {
#ifdef BUILD_LIVECODING
		if (livecoding_is_running()) {
			engine_wait_next_reload();
			lua_pop(L, 1);
		} else
#endif
		{
			engine_free();
			exit(EXIT_FAILURE);
		}
	}
	lua_remove(L, base);
#endif
}

