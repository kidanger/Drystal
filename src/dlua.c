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
#include <math.h>
#include <assert.h>
#include <stdbool.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "lua_util.h"
#include "engine.h"
#include "util.h"
#include "log.h"
#include "dlua.h"
#include "luafiles.h"
#include "module.h"
#ifdef BUILD_PHYSICS
#include "physics/api.hpp"
#endif
#ifdef BUILD_FONT
#include "truetype/api.h"
#endif
#ifdef BUILD_PARTICLE
#include "particle/api.h"
#endif
#ifdef BUILD_AUDIO
#include "audio/api.h"
#endif
#ifdef BUILD_WEB
#include "web/api.h"
#endif
#ifdef BUILD_STORAGE
#include "storage/api.h"
#endif
#ifdef BUILD_GRAPHICS
#include "event/api.h"
#include "graphics/api.h"
#endif
#ifdef BUILD_UTILS
#include "utils/api.h"
#endif
#ifdef BUILD_LIVECODING
#include "livecoding.h"
#ifdef BUILD_GRAPHICS
#include "graphics/display.h"
#endif
#ifdef BUILD_AUDIO
#include "audio/audio.h"
#endif
#endif

log_category("lua");

static int luaopen_drystal(lua_State*); // defined at the end of this file

struct DrystalLua {
	lua_State* L;
	int drystal_table_ref;
	bool library_loaded;
	const char* filename;
} dlua;

void dlua_init(const char *filename)
{
	dlua.L = luaL_newstate();
	dlua.drystal_table_ref = LUA_NOREF;
	dlua.filename = filename;
	dlua.library_loaded = false;
	luaL_openlibs(dlua.L);
}

void dlua_free(void)
{
	luaL_unref(dlua.L, LUA_REGISTRYINDEX, dlua.drystal_table_ref);
	lua_close(dlua.L);
}

lua_State *dlua_get_lua_state(void)
{
	return dlua.L;
}

void dlua_get_drystal_field(const char* name)
{
	lua_State *L = dlua.L;

	assert(name);

	lua_rawgeti(L, LUA_REGISTRYINDEX, dlua.drystal_table_ref);
	lua_pushstring(L, name);
	lua_rawget(L, -2);
	lua_remove(L, lua_gettop(L) - 1);
}

/**
 * Search for a function named 'name' in the drystal table.
 * Return true if found and keep the function in the lua stack
 * Otherwise, return false (stack is cleaned as needed).
 */
bool dlua_get_function(const char* name)
{
	lua_State *L = dlua.L;

	assert(name);

	dlua_get_drystal_field(name);
	if (lua_isfunction(L, -1)) {
		return true;
	}
	lua_pop(L, 1);
	return false;
}

static void register_modules(void)
{
#ifdef BUILD_AUDIO
	REGISTER_MODULE(audio, dlua.L);
#endif
#ifdef BUILD_PARTICLE
	REGISTER_MODULE(particle, dlua.L);
#endif
#ifdef BUILD_PHYSICS
	REGISTER_MODULE(physics, dlua.L);
#endif
#ifdef BUILD_STORAGE
	REGISTER_MODULE(storage, dlua.L);
#endif
#ifdef BUILD_FONT
	REGISTER_MODULE(truetype, dlua.L);
#endif
#ifdef BUILD_WEB
	REGISTER_MODULE(web, dlua.L);
#endif
#ifdef BUILD_GRAPHICS
	REGISTER_MODULE(graphics, dlua.L);
	REGISTER_MODULE(event, dlua.L);
#endif
#ifdef BUILD_UTILS
	REGISTER_MODULE(utils, dlua.L);
#endif
}

bool dlua_load_code(void)
{
	lua_State *L = dlua.L;

	assert(lua_gettop(L) == 0);
	if (!dlua.library_loaded) {
		// add drystal lib
		luaL_requiref(L, "drystal", luaopen_drystal, 0 /* not as global */);
		register_modules();
		lua_pop(L, 1);  /* remove lib */

		lua_pushcfunction(L, traceback);
		if (!load_luafiles(L, lua_gettop(L))) {
			lua_pop(L, 2);
			assert(lua_gettop(L) == 0);
			return false;
		}
		lua_pop(L, 1);
		dlua.library_loaded = true;
	}

	if (luaL_loadfile(L, dlua.filename)) {
		fprintf(stderr, "*** ERROR ***\n%s\n", lua_tostring(L, -1));
		lua_pop(L, 1);
		assert(lua_gettop(L) == 0);
#ifdef BUILD_LIVECODING
		if (livecoding_is_running()) {
			engine_wait_next_reload();
		}
#endif
		return false;
	}

	call_lua_function(L, 0, 0);

	assert(lua_gettop(L) == 0);
	return true;
}

bool dlua_foreach(const char* type, bool(*callback)(void* data, const void* callback_arg), const void* callback_arg)
{
	assert(type);
	assert(callback);

	bool result = false;
	lua_State *L = dlua.L;
	lua_getfield(L, LUA_REGISTRYINDEX, "objects");
	lua_pushnil(L);
	while (lua_next(L, -2)) {
		if (lua_istable(L, -1)) {
			lua_getfield(L, -1, "__self");
			lua_replace(L, -2);
		}

		lua_getfield(L, -1, "__type");
		const char* t = lua_tostring(L, -1);
		lua_pop(L, 1);
		if (streq(type, t)) {
			void* data = * (void**) lua_touserdata(L, -1);
			result |= callback(data, callback_arg);
		}

		lua_pop(L, 1);
	}
	lua_pop(L, 1);
	return result;
}

void dlua_call_init(void)
{
	if (dlua_get_function("init")) {
		call_lua_function(dlua.L, 0, 0);
	}
}

void dlua_call_update(float dt)
{
	if (dlua_get_function("update")) {
		lua_State *L = dlua.L;
		lua_pushnumber(L, dt);
		call_lua_function(L, 1, 0);
	}
}

void dlua_call_draw(void)
{
	if (dlua_get_function("draw")) {
		call_lua_function(dlua.L, 0, 0);
	}
}

void dlua_call_atexit(void)
{
	if (dlua_get_function("atexit")) {
		call_lua_function(dlua.L, 0, 0);
	}
}

bool dlua_reload_code(void)
{
	lua_State* L = dlua.L;
	assert(L);

	dlua_get_function("reload");
	call_lua_function(L, 0, 1);
	bool ok = lua_toboolean(L, -1);
	lua_pop(L, 1);
	return ok;
}

static int mlua_stop(_unused_ lua_State *L)
{
	engine_stop();
	return 0;
}

static int mlua_load_code(lua_State *L)
{
	bool ok = dlua_load_code();
	lua_pushboolean(L, ok);
	return 1;
}

#ifdef BUILD_LIVECODING
#ifdef BUILD_AUDIO
static bool reload_sound(void* data, const void* arg)
{
	const char* filename = arg;
	Sound* s = data;
	Sound* new_sound;
	int r;

	if (strcmp(s->filename, filename))
		return false;

	r = sound_load_from_file(s->filename, &new_sound);
	if (r < 0)
		return false;

	SWAP(s->alBuffer, new_sound->alBuffer);
	SWAP(s->free_me, new_sound->free_me);
	sound_free(new_sound);

	log_debug("%s reloaded", s->filename);
	return true;
}
static int mlua_reload_sound(lua_State *L)
{
	assert(L);

	const char* filename = lua_tostring(L, 1);
	bool done = dlua_foreach("sound", reload_sound, filename);
	lua_pushboolean(L, done);
	return 1;
}
#endif

#ifdef BUILD_GRAPHICS
static bool reload_surface(void* data, const void* arg)
{
	const char* filename = arg;
	Surface* s = data;
	Surface* new_surface;

	if (!s->filename || strcmp(s->filename, filename))
		return false;

	if (display_load_surface(s->filename, &new_surface))
		return false;

	FilterMode filter = s->filter;
	SWAP(s->w, new_surface->w);
	SWAP(s->h, new_surface->h);
	SWAP(s->texw, new_surface->texw);
	SWAP(s->texh, new_surface->texh);
	SWAP(s->has_fbo, new_surface->has_fbo);
	SWAP(s->has_mipmap, new_surface->has_mipmap);
	SWAP(s->npot, new_surface->npot);
	SWAP(s->tex, new_surface->tex);
	SWAP(s->fbo, new_surface->fbo);
	display_free_surface(new_surface);

	display_set_filter(s, filter);
	log_debug("%s reloaded", s->filename);
	return true;
}
static int mlua_reload_surface(lua_State *L)
{
	assert(L);

	const char* filename = lua_tostring(L, 1);
	bool done = dlua_foreach("surface", reload_surface, filename);
	lua_pushboolean(L, done);
	return 1;
}
#endif
#endif

static int mlua_drystal_index(_unused_ lua_State *L)
{
#ifdef BUILD_GRAPHICS
	int r;

	r = graphics_index(dlua.L);
	if (r > 0) {
		return r;
	}
#endif
	return 0;
}

int luaopen_drystal(lua_State *L)
{
	assert(L);

	lua_newtable(L);
	luaL_newmetatable(L, "__objects");
	lua_pushliteral(L, "v");
	lua_setfield(L, -2, "__mode");
	lua_setmetatable(L, -2);
	lua_setfield(L, LUA_REGISTRYINDEX, "objects");

	static const luaL_Reg lib[] = {
		{"stop", mlua_stop},
		{"_load_code", mlua_load_code},
#ifdef BUILD_LIVECODING
#ifdef BUILD_AUDIO
		{"reload_sound", mlua_reload_sound},
#endif
#ifdef BUILD_GRAPHICS
		{"reload_surface", mlua_reload_surface},
#endif
#endif
		{NULL, NULL}
	};

	luaL_newlib(L, lib);

	luaL_newmetatable(L, "_drystal");
	lua_pushcfunction(L, mlua_drystal_index);
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, -2);

	lua_pushvalue(L, -1);
	dlua.drystal_table_ref = luaL_ref(L, LUA_REGISTRYINDEX);

	assert(lua_gettop(L) == 2);
	return 1;
}

