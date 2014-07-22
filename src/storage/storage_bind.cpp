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

#include "lua_functions.hpp"
#include "storage_bind.hpp"

extern "C" {
	extern int json_encode(lua_State* L);
	extern int json_decode(lua_State* L);
	extern void lua_cjson_init();
}

#ifdef EMSCRIPTEN

#include <emscripten.h>
#include <string>

static const char* fetch(const char* key)
{
	assert(key);

	std::string js;
	js = "if (localStorage!==undefined) {localStorage['";
	js += key;
	js += "']||''} else {''}";

	const char* value = emscripten_run_script_string(js.c_str());
	return value;
}

static void store(const char* key, const char* value)
{
	assert(key);
	assert(value);

	std::string js;
	js = "localStorage['";
	js += key;
	js += "'] = '";
	js += value;
	js += "';";

	emscripten_run_script(js.c_str());
}

#else
#include <cstdio>
#include <cstring>

#include "macro.hpp"

static char data[1024] = {0};

static const char* fetch(_unused_ const char* key)
{
	FILE* file = fopen(".storage", "r");
	if (file == NULL)
		return "";
	fread(data, sizeof(data), 1, file);
	fclose(file);
	return data;
}

static void store(_unused_ const char* key, const char* value)
{
	assert(value);

	FILE* file = fopen(".storage", "w");
	fwrite(value, strlen(value), 1, file);
	fclose(file);
}

#endif

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
	const char* value = fetch(key);

	if (!value[0]) {
		return 0;
	}

	lua_pushcfunction(L, json_decode);
	lua_pushstring(L, value);
	CALL(1, 1);
	// table is returned by json_decode
	return 1;
}
