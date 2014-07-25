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
#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#include "web.hpp"
#include "engine.hpp"
#include "macro.hpp"

bool is_web()
{
#ifdef EMSCRIPTEN
	return true;
#else
	return false;
#endif
}

#ifdef EMSCRIPTEN
void run_js(const char* script)
{
	emscripten_run_script(script);
}
#else
void run_js(_unused_ const char* script)
{
}
#endif

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

void wget(const char* url, const char* filename)
{
	emscripten_async_wget(url, filename, onsuccess, onerror);
}
#else
void wget(_unused_ const char* url, _unused_ const char* filename)
{
}
#endif
