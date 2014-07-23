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

#define PUSH_FUNC(name, func) \
	lua_pushcfunction(L, mlua_##func); \
	lua_setfield(L, -2, name);

#define BEGIN_MODULE(name) \
	static inline void register_##name(lua_State* L) {
#define DECLARE_FUNCTION(name) \
	lua_pushcfunction(L, mlua_##name); \
	lua_setfield(L, -2, #name);
#define END_MODULE() \
	}

#define BEGIN_CLASS(name) \
	luaL_newmetatable(L, #name);
#define ADD_GC(func) \
	PUSH_FUNC("__gc", func)
#define ADD_METHOD(class, name) \
	PUSH_FUNC(#name, name##_##class)
#define ADD_GETSET(class, name) \
	ADD_METHOD(class, get_##name) \
	ADD_METHOD(class, set_##name)

#define REGISTER_CLASS(name, name_in_module) \
	lua_pushvalue(L, -1); \
	lua_setfield(L, -2, "__index"); \
	lua_setfield(L, -2, name_in_module);

#define REGISTER_CLASS_WITH_INDEX(name, name_in_module) \
	PUSH_FUNC("__index", name##_class_index); \
	lua_setfield(L, -2, name_in_module);

#define REGISTER_CLASS_WITH_INDEX_AND_NEWINDEX(name, name_in_module) \
	PUSH_FUNC("__index", name##_class_index); \
	PUSH_FUNC("__newindex", name##_class_newindex); \
	lua_setfield(L, -2, name_in_module);

#define REGISTER_MODULE(name, L) \
	register_##name(L)
