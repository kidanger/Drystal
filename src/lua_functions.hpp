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

#include <cassert>
#include <lua.hpp>

class LuaFunctions
{
public:
	lua_State* L;
	int drystal_table_ref;

	LuaFunctions(const char *_filename);

	bool load_code();
	bool reload_code();

	bool call_init() const;
	void call_update(float dt) const;
	void call_draw() const;
	void call_atexit() const;

	bool get_function(const char* name) const;
	void free();

private:
	const char* filename;
	bool library_loaded;

	LuaFunctions(const LuaFunctions&);
	LuaFunctions& operator=(const LuaFunctions&);
	void remove_userpackages() const;
	void register_modules();
};

#define DECLARE_PUSH(T, name) \
	static void push_ ## name(lua_State *L, T *name) \
	{ \
		assert(L); \
		assert(name); \
		T **p = static_cast<T **>(lua_newuserdata(L, sizeof(T **))); \
		*p = name; \
		luaL_getmetatable(L, #name); \
		lua_setmetatable(L, -2); \
	}

#define DECLARE_POP(T, name) \
	static T *pop_ ## name(lua_State *L, int index) \
	{ \
		assert(L); \
		if (!lua_istable(L, index)) { \
			T **p = static_cast<T **>(lua_touserdata(L, index)); \
			if (p == NULL) luaL_argerror(L, index, #name" expected"); \
			return *p; \
		} else { \
			lua_getfield(L, index, "__self"); \
			T* p = pop_##name(L, -1); \
			lua_pop(L, 1); \
			return p; \
		} \
	}

#define DECLARE_PUSH2(T, name) \
	static void push_ ## name(lua_State *L, T *name) \
	{ \
		assert(L); \
		assert(name); \
		lua_getfield(L, LUA_REGISTRYINDEX, "objects"); \
		if (name->ref) { \
			lua_rawgeti(L, -1, name->ref); \
		} else { \
			T **p = static_cast<T **>(lua_newuserdata(L, sizeof(T **))); \
			*p = name; \
			\
			lua_newtable(L); /* storage */ \
			lua_pushvalue(L, -1); \
			lua_setfield(L, -2, "__index"); \
			lua_pushvalue(L, -1); \
			lua_setfield(L, -2, "__newindex"); \
			lua_pushvalue(L, -2); /* used to retrieve the userdata in class.__index function */ \
			lua_setfield(L, -2, "__self"); \
			\
			luaL_setmetatable(L, #name); /* setmetatable(storage, class) */ \
			lua_setmetatable(L, -2); /* setmetatable(userdata, storage) */ \
			\
			lua_pushvalue(L, -1); \
			name->ref = luaL_ref(L, -3); \
		} \
		lua_remove(L, lua_gettop(L) - 1); \
	}

#define DECLARE_PUSHPOP(T, name) \
	DECLARE_PUSH(T, name) \
	DECLARE_POP(T, name)
#define DECLARE_PUSHPOP2(T, name) \
	DECLARE_PUSH2(T, name) \
	DECLARE_POP(T, name)

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

