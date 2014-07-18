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
#pragma once

#ifndef EMSCRIPTEN
#include <ctime>
#endif

#include "engine.hpp"

struct lua_State;

class LuaFunctions
{
public:
	lua_State* L;
	int drystal_table_ref;

	LuaFunctions(Engine&, const char *_filename);
	~LuaFunctions();

	void add_search_path(const char* path) const;

	bool load_code();
	bool reload_code();
	bool call_init() const;

	void call_update(float dt) const;
	void call_draw() const;

	void call_resize_event(int w, int h) const;
	void call_mouse_motion(int mx, int my, int dx, int dy) const;
	void call_mouse_press(int mx, int my, int button) const;
	void call_mouse_release(int mx, int my, int button) const;
	void call_key_press(const char* key_string) const;
	void call_key_release(const char* key_string) const;
	void call_key_text(const char* string) const;
	void call_atexit() const;

private:
	const char* filename;
	bool library_loaded;

	LuaFunctions(const LuaFunctions&);
	LuaFunctions& operator=(const LuaFunctions&);
	bool get_function(const char* name) const;
	void remove_userpackages() const;
	void register_modules();
};

#define DECLARE_PUSHPOP(T, name) \
	static void push_ ## name(lua_State *L, T *name) \
	{ \
		assert(L); \
		assert(name); \
		T **p = static_cast<T **>(lua_newuserdata(L, sizeof(T **))); \
		*p = name; \
		luaL_getmetatable(L, #name); \
		lua_setmetatable(L, -2); \
	} \
	static T *pop_ ## name(lua_State *L, int index) \
	{ \
		assert(L); \
		luaL_checktype(L, index, LUA_TUSERDATA); \
		T **p = static_cast<T **>(luaL_checkudata(L, index, #name)); \
		if (p == NULL) luaL_argerror(L, index, #name" expected"); \
		assert(p); \
		return *p; \
	}


extern int traceback(lua_State *L);
#ifdef EMSCRIPTEN
#define CALL(num_args, num_ret) \
	lua_call(L, num_args, num_ret);
#else
#define CALL(num_args, num_ret) \
	/* from lua/src/lua.c */ \
	int base = lua_gettop(L) - num_args; \
	lua_pushcfunction(L, traceback); \
	lua_insert(L, base);  \
	if (lua_pcall(L, num_args, num_ret, base)) { \
		luaL_error(L, "%s: %s", __func__, lua_tostring(L, -1)); \
	} \
	lua_remove(L, base);
#endif

