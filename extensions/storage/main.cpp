#include <cstring>
#include <lua.hpp>

#include "engine.hpp"

extern "C" {
	extern int json_encode(lua_State* L);
	extern int json_decode(lua_State* L);
	extern void lua_cjson_init();
}

#ifdef EMSCRIPTEN
#include "emscripten.h"

const char* storage_fetch(const char* key)
{
	const char js[] = "if (localStorage!==undefined) {localStorage.%s||''} else {''}";

	int size = strlen(js) + strlen(key);
	char* buf = new char[size];
	snprintf(buf, size, js, key);

	const char* value = emscripten_run_script_string(buf);
	delete[] buf;

	return value;
}
void storage_store(const char* key, const char* value)
{
	const char js[] = "localStorage.%s = '%s';";

	int size = strlen(js) + strlen(key) + strlen(value);
	char* buf = new char[size];
	snprintf(buf, size, js, key, value);

	emscripten_run_script(buf);
	delete[] buf;
}
#else

char data[1024] = {0};

const char* storage_fetch(const char* key)
{
	(void) key;
	FILE* file = fopen(".storage", "r");
	if (not file)
		return "";
	fread(data, sizeof(data), 1, file);
	fclose(file);
	return data;
}
void storage_store(const char* key, const char* value)
{
	(void) key;
	FILE* file = fopen(".storage", "w");
	fwrite(value, strlen(value), 1, file);
	fclose(file);
}
#endif

int storage_save(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);

	lua_pushcfunction(L, json_encode);
	lua_pushvalue(L, 2);
	if (lua_pcall(L, 1, 1, 0)) { // table in param, returns json
		luaL_error(L, "error calling storage.load: %s\n", lua_tostring(L, -1));
	}

	const char* value = luaL_checkstring(L, -1);
	storage_store(key, value);
	return 0;
}

int storage_load(lua_State* L)
{
	const char* key = luaL_checkstring(L, 1);
	const char* value = storage_fetch(key);

	if (!value[0]) {
		return 0;
	}

	lua_pushcfunction(L, json_decode);
	lua_pushstring(L, value);
	if (lua_pcall(L, 1, 1, 0)) {
		luaL_error(L, "error calling storage.load: %s\n", lua_tostring(L, -1));
	}

	return 1;
}

static const luaL_Reg lib[] =
{
	{"save", storage_save},
	{"load", storage_load},
	{NULL, NULL}
};

DEFINE_EXTENSION(storage)
{
	luaL_newlib(L, lib);

	lua_cjson_init();

	return 1;
}

