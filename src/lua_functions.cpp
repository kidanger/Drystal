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
#include <cstring>
#include <cmath>
#include <cassert>

#include <lua.hpp>

#include "lua_util.hpp"
#include "engine.hpp"
#include "log.hpp"
#include "lua_functions.hpp"
#include "luafiles.hpp"
#include "module.hpp"
#ifdef BUILD_EVENT
#include "event/api.hpp"
#endif
#ifdef BUILD_PHYSICS
#include "physics/api.hpp"
#endif
#ifdef BUILD_FONT
#include "truetype/api.hpp"
#endif
#ifdef BUILD_PARTICLE
#include "particle/api.hpp"
#endif
#ifdef BUILD_AUDIO
#include "audio/api.hpp"
#endif
#ifdef BUILD_NET
#include "net/api.hpp"
#endif
#ifdef BUILD_WEB
#include "web/api.hpp"
#endif
#ifdef BUILD_STORAGE
#include "storage/api.hpp"
#endif
#ifdef BUILD_GRAPHICS
#include "graphics/api.hpp"
#endif
#ifdef BUILD_UTILS
#include "utils/api.hpp"
#endif

log_category("lua");

static int luaopen_drystal(lua_State*); // defined at the end of this file

LuaFunctions::LuaFunctions(const char *_filename) :
	L(luaL_newstate()),
	drystal_table_ref(LUA_NOREF),
	filename(_filename),
#ifdef BUILD_LIVECODING
	need_to_reload(false),
#endif
	library_loaded(false)
{
	luaL_openlibs(L);
}

void LuaFunctions::free()
{
	luaL_unref(L, LUA_REGISTRYINDEX, drystal_table_ref);
	lua_close(L);
}

/**
 * Search for a function named 'name' in the drystal table.
 * Return true if found and keep the function in the lua stack
 * Otherwise, return false (stack is cleaned as needed).
 */
bool LuaFunctions::get_function(const char* name) const
{
	assert(name);

	lua_rawgeti(L, LUA_REGISTRYINDEX, drystal_table_ref);
	lua_pushstring(L, name);
	lua_rawget(L, -2);
	if (lua_isfunction(L, -1)) {
		lua_remove(L, lua_gettop(L) - 1);
		return true;
	}
	lua_pop(L, 2);
	return false;
}

void LuaFunctions::remove_userpackages() const
{
	assert(lua_gettop(L) == 0);
	log_info("Removing old packages: ");
	const char* kept[] = {
		"_G",
		LUA_COLIBNAME,
		LUA_TABLIBNAME,
		LUA_IOLIBNAME,
		LUA_OSLIBNAME,
		LUA_STRLIBNAME,
		LUA_BITLIBNAME,
		LUA_MATHLIBNAME,
		LUA_DBLIBNAME,
		LUA_LOADLIBNAME,
		"drystal",
	};
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "loaded");
	lua_pushnil(L);
	while (lua_next(L, -2)) {
		bool remove = true;
		const char* name = lua_tostring(L, -2);
		for (unsigned long i = 0; i < sizeof(kept) / sizeof(const char*) && remove; i++) {
			remove = remove && strcmp(name, kept[i]);
		}
		if (remove) {
			lua_pushnil(L);
			log_info("    Removed %s", name);
			lua_setfield(L, -4, name);
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 2);
	assert(lua_gettop(L) == 0);
}

bool LuaFunctions::load_code()
{
	assert(lua_gettop(L) == 0);
	lua_pushcfunction(L, traceback); // used by lua_pcall
	if (!library_loaded) {
		// add drystal lib
		luaL_requiref(L, "drystal", luaopen_drystal, 0 /* not as global */);
		register_modules();
		lua_pop(L, 1);  /* remove lib */

		if (!load_luafiles(L)) {
			log_error("Cannot run script: %s", lua_tostring(L, -1));
			lua_pop(L, 2);
			return false;
		}
		library_loaded = true;
	}

	if (luaL_loadfile(L, filename) || lua_pcall(L, 0, 0, -2)) {
		log_error("Cannot run script: %s", lua_tostring(L, -1));
		lua_pop(L, 2);
		return false;
	}

	lua_pop(L, 1);
	assert(lua_gettop(L) == 0);
	return true;
}

#ifdef BUILD_LIVECODING
void LuaFunctions::set_need_to_reload()
{
	need_to_reload = true;
}

std::atomic<bool>& LuaFunctions::is_need_to_reload()
{
	return need_to_reload;
}
#endif

bool LuaFunctions::reload_code()
{
	if (get_function("prereload")) {
		CALL(0, 0);
	}
	remove_userpackages();

	log_info("Reloading code...");
	bool ok = load_code() && call_init();
	if (ok) {
		if (get_function("postreload")) {
			CALL(0, 0);
		}
	}

#ifdef BUILD_LIVECODING
	need_to_reload = false;
#endif

	return ok;
}

bool LuaFunctions::call_init() const
{
	if (get_function("init")) {
		CALL(0, 0);
	}
	return true;
}

void LuaFunctions::call_update(float dt) const
{
	if (get_function("update")) {
		lua_pushnumber(L, dt);
		CALL(1, 0);
	}
}

void LuaFunctions::call_draw() const
{
	if (get_function("draw")) {
		CALL(0, 0);
	}
}

void LuaFunctions::call_atexit() const
{
	if (get_function("atexit")) {
		CALL(0, 0);
	}
}

static int mlua_stop(lua_State*)
{
	Engine &engine = get_engine();
	engine.stop();
	return 0;
}

static int mlua_reload(lua_State*)
{
	Engine &engine = get_engine();
	engine.lua.reload_code();
	return 0;
}

#ifdef BUILD_GRAPHICS
static int mlua_drystal_index(lua_State* L)
#else
static int mlua_drystal_index(_unused_ lua_State* L)
#endif
{
#ifdef BUILD_GRAPHICS
	int r;

	assert(L);

	r = graphics_index(L);
	if (r > 0) {
		return r;
	}
#endif
	return 0;
}

void LuaFunctions::register_modules()
{
#ifdef BUILD_AUDIO
	REGISTER_MODULE(audio, L);
#endif
#ifdef BUILD_EVENT
	REGISTER_MODULE(event, L);
#endif
#ifdef BUILD_NET
	REGISTER_MODULE(net, L);
#endif
#ifdef BUILD_PARTICLE
	REGISTER_MODULE(particle, L);
#endif
#ifdef BUILD_PHYSICS
	REGISTER_MODULE(physics, L);
#endif
#ifdef BUILD_STORAGE
	REGISTER_MODULE(storage, L);
#endif
#ifdef BUILD_FONT
	REGISTER_MODULE(truetype, L);
#endif
#ifdef BUILD_WEB
	REGISTER_MODULE(web, L);
#endif
#ifdef BUILD_GRAPHICS
	REGISTER_MODULE(graphics, L);
#endif
#ifdef BUILD_UTILS
	REGISTER_MODULE(utils, L);
#endif
}

int luaopen_drystal(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();

	lua_newtable(L);
	luaL_newmetatable(L, "__objects");
	lua_pushstring(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_REGISTRYINDEX, "objects");

#define EXPOSE_FUNCTION(name) {#name, mlua_##name}
	static const luaL_Reg lib[] = {
		EXPOSE_FUNCTION(stop),
		EXPOSE_FUNCTION(reload),

		{NULL, NULL}
	};
#undef EXPOSE_FUNCTION

	luaL_newlib(L, lib);

	luaL_newmetatable(L, "_drystal");
	lua_pushcfunction(L, mlua_drystal_index);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);

	lua_pushvalue(L, -1);
	engine.lua.drystal_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	assert(lua_gettop(L) == 2);
	return 1;
}
