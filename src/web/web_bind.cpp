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

	lua_State* L = get_engine().lua.L;
	lua_getglobal(L, "on_wget_success");
	if (not lua_isfunction(L, -1)) {
		lua_pop(L, 1);
		return;
	}
	lua_pushstring(L, filename);
	if (lua_pcall(L, 1, 0, 0)) {
		luaL_error(L, "error calling on_wget_success: %s", lua_tostring(L, -1));
	}
}

static void onerror(const char* filename)
{
	assert(filename);

	lua_State* L = get_engine().lua.L;
	lua_getglobal(L, "on_wget_error");
	if (not lua_isfunction(L, -1)) {
		lua_pop(L, 1);
		return;
	}
	lua_pushstring(L, filename);
	if (lua_pcall(L, 1, 0, 0)) {
		luaL_error(L, "error calling on_wget_error: %s", lua_tostring(L, -1));
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
