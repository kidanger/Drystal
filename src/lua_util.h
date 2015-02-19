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

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include <lua.h>
#include <lauxlib.h>

#include "log.h"

int traceback(lua_State *L);
void call_lua_function(lua_State *L, int num_args, int num_ret);

#define DECLARE_PUSH(T, name) \
	void push_ ## name(lua_State *L, T *name);
#define DECLARE_POP(T, name) \
	T *pop_ ## name(lua_State *L, int index);

#define IMPLEMENT_PUSH(T, name) \
	void push_ ## name(lua_State *L, T *name) \
	{ \
		assert(L); \
		assert(name); \
		lua_getfield(L, LUA_REGISTRYINDEX, "objects"); \
		if (name->ref) { \
			lua_rawgeti(L, -1, name->ref); \
		} else { \
			T **p = (T **) lua_newuserdata(L, sizeof(T **)); \
			if (!p) \
				log_oom_and_exit(); \
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

#define IMPLEMENT_POP(T, name) \
	T *pop_ ## name(lua_State *L, int index) \
	{ \
		assert(L); \
		if (!lua_istable(L, index)) { \
			T **p = (T **) lua_touserdata(L, index); \
			if (p == NULL) luaL_argerror(L, index, #name" expected"); \
			assert(p); \
			return *p; \
		} else { \
			lua_getfield(L, index, "__self"); \
			T* p = pop_##name(L, -1); \
			lua_pop(L, 1); \
			return p; \
		} \
	}

#define DECLARE_PUSHPOP(T, name) \
	DECLARE_PUSH(T, name) \
	DECLARE_POP(T, name)

#define IMPLEMENT_PUSHPOP(T, name) \
	IMPLEMENT_PUSH(T, name) \
	IMPLEMENT_POP(T, name)

#define assert_lua_error(L, x, msg) \
	if (!(x)) \
		luaL_error(L, msg)

#ifdef __cplusplus
}
#endif

