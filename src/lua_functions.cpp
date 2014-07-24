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

#include "engine.hpp"
#include "log.hpp"
#include "lua_functions.hpp"
#include "luafiles.hpp"
#include "module.hpp"
#ifdef BUILD_EVENT
#include "event/api.hpp"
#endif
#ifdef BUILD_PHYSIC
#include "physic/api.hpp"
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

log_category("lua");

static int luaopen_drystal(lua_State*); // defined at the end of this file

LuaFunctions::LuaFunctions(const char *_filename) :
	L(luaL_newstate()),
	drystal_table_ref(LUA_NOREF),
	filename(_filename),
	library_loaded(false)
{
	luaL_openlibs(L);
}

void LuaFunctions::free()
{
	luaL_unref(L, LUA_REGISTRYINDEX, drystal_table_ref);
	lua_close(L);
}

int traceback(lua_State *L)
{
	// from lua/src/lua.c
	const char *msg = lua_tostring(L, 1);
	if (msg)
		luaL_traceback(L, L, msg, 1);
	else if (!lua_isnoneornil(L, 1)) {  /* is there an error object? */
		if (!luaL_callmeta(L, 1, "__tostring"))  /* try its 'tostring' metamethod */
			lua_pushliteral(L, "(no error message)");
	}
	return 1;
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
	assert(L);
#ifdef BUILD_GRAPHICS
	Engine &engine = get_engine();
	const char * name = luaL_checkstring(L, 2);
	if (!strcmp(name, "screen")) {
		Surface* surf = engine.display.get_screen();
		if (surf) {
			push_surface(L, surf);
			return 1;
		}
	} else if (!strcmp(name, "current_draw_on")) {
		Surface* surf = engine.display.get_draw_on();
		if (surf) {
			push_surface(L, surf);
			return 1;
		}
	} else if (!strcmp(name, "current_draw_from")) {
		Surface* surf = engine.display.get_draw_from();
		if (surf) {
			push_surface(L, surf);
			return 1;
		}
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
#ifdef BUILD_PHYSIC
	REGISTER_MODULE(physic, L);
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
}

extern "C" {
	extern int json_encode(lua_State* L);
	extern int json_decode(lua_State* L);
	extern void lua_cjson_init();
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
		{"engine_stop", mlua_stop},
		EXPOSE_FUNCTION(stop),
		EXPOSE_FUNCTION(reload),

		/* SERIALIZER */
		{"serialize", json_encode},
		{"deserialize", json_decode},

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

	lua_cjson_init();

	assert(lua_gettop(L) == 2);
	return 1;
}
