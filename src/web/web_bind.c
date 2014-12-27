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
#include <lua.h>
#include <lauxlib.h>
#include <assert.h>
#ifdef EMSCRIPTEN
#include <stdlib.h>
#endif

#include "web.h"
#include "web_bind.h"
#include "log.h"

int mlua_run_js(lua_State* L)
{
	assert(L);

#ifdef EMSCRIPTEN
	const char* script = luaL_checkstring(L, 1);
	char *ret = run_js(script);
	lua_pushstring(L, ret);
	free(ret);
	return 1;
#else
	return luaL_error(L, "run_js isn't available in native build");
#endif
}

int mlua_wget(lua_State *L)
{
	assert(L);

#ifdef EMSCRIPTEN
	const char *url = luaL_checkstring(L, 1);
	const char *filename = luaL_checkstring(L, 2);
	wget(url, filename);
	return 0;
#else
	return luaL_error(L, "wget isn't available in native build");
#endif
}

