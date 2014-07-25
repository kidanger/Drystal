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
#include <lua.hpp>
#include <cassert>

#include "engine.hpp"
#include "web_bind.hpp"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

int mlua_is_web(lua_State* L)
{
	assert(L);
#ifdef EMSCRIPTEN
	lua_pushboolean(L, 1);
#else
	lua_pushboolean(L, 0);
#endif
	return 1;
}

#ifdef EMSCRIPTEN
static void onsuccess(const char* filename)
{
	assert(filename);

	Engine& engine = get_engine();
	if (engine.lua.get_function("on_wget_success")) {
		lua_State* L = engine.lua.L;
		lua_pushstring(L, filename);
		CALL(1, 0);
	}
}

static void onerror(const char* filename)
{
	assert(filename);

	Engine& engine = get_engine();
	if (engine.lua.get_function("on_wget_error")) {
		lua_State* L = engine.lua.L;
		lua_pushstring(L, filename);
		CALL(1, 0);
	}
}

int mlua_wget(lua_State *L)
{
	assert(L);

	const char *url = luaL_checkstring(L, 1);
	const char *filename = luaL_checkstring(L, 2);
	emscripten_async_wget(url, filename, onsuccess, onerror);
	lua_pushboolean(L, true);
	return 1;
}

#else
int mlua_wget(lua_State *L)
{
	assert(L);

	lua_pushboolean(L, false);
	return 1;
}
#endif

int mlua_run_js(lua_State* L)
{
	assert(L);

#ifdef EMSCRIPTEN
	const char* script = luaL_checkstring(L, 1);
	emscripten_run_script(script);
#else
	(void) L;
#endif
	return 0;
}
