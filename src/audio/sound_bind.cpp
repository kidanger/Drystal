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
#include <lua.hpp>

#include "engine.hpp"
#include "log.hpp"
#include "sound_bind.hpp"
#include "sound.hpp"

log_category("sound");

DECLARE_PUSHPOP(Sound, sound)

int mlua_load_sound(lua_State *L)
{
	assert(L);

    if (lua_isstring(L, 1)) {
		const char* filename = lua_tostring(L, 1);
		Sound *chunk = Sound::load_from_file(filename);
		if (chunk) {
			push_sound(L, chunk);
			return 1;
		}
		return luaL_fileresult(L, 0, filename);
	} else {
		/*
		 * Multiple configurations allowed:
		 * [1]: table
		 * 	len = #table (can call __len)
		 * 	data = table[i] (can call __index)
		 * or
		 * [1]: table
		 * [2]: number
		 * 	len = number
		 * 	data = table[i] (can call __index)
		 * or
		 * [1]: function
		 * [2]: number
		 * 	len = number
		 * 	data = function(i)
		 */
		unsigned int len;
		if (lua_gettop(L) == 1) {
			len = luaL_len(L, 1);
		} else {
			len = luaL_checknumber(L, 2);
		}

		float buffer[len];
		if (lua_istable(L, 1)) {
			for (unsigned int i = 0; i < len; i++) {
				lua_pushnumber(L, i + 1);
				lua_gettable(L, 1);
				buffer[i] = luaL_checknumber(L, -1);
				lua_pop(L, 1);
			}
		} else if (lua_isfunction(L, 1)) {
			for (unsigned int i = 0; i < len; i++) {
				lua_pushvalue(L, 1);
				lua_pushnumber(L, i);
				lua_call(L, 1, 1);
				buffer[i] = luaL_checknumber(L, -1);
				lua_pop(L, 1);
			}
		}

		Sound *chunk = Sound::load(len, buffer);
		push_sound(L, chunk);
		return 1;
	}
	return 0;
}

int mlua_play_sound(lua_State *L)
{
	assert(L);

	Sound* sound = pop_sound(L, 1);

	float volume = 1;
	float x = 0;
	float y = 0;
	if (!lua_isnone(L, 2))
		volume = luaL_checknumber(L, 2);
	if (!lua_isnone(L, 3))
		x = luaL_checknumber(L, 3);
	if (!lua_isnone(L, 4))
		y = luaL_checknumber(L, 4);

	sound->play(volume, x, y);
	return 0;
}

int mlua_free_sound(lua_State *L)
{
	assert(L);

	log_debug("");
	Sound* sound = pop_sound(L, 1);
	sound->free();
	return 0;
}

