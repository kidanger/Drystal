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

#include "audio_bind.hpp"
#include "lua_util.h"
#include "audio.hpp"

int mlua_set_sound_volume(lua_State *L)
{
	assert(L);

	float volume = luaL_checknumber(L, 1);

	assert_lua_error(L, volume >= 0 && volume <= 1, "set_sound_volume: must be >= 0 and <= 1");

	set_sound_volume(volume);
	return 0;
}

int mlua_set_music_volume(lua_State *L)
{
	assert(L);

	float volume = luaL_checknumber(L, 1);

	assert_lua_error(L, volume >= 0 && volume <= 1, "set_music_volume: must be >= 0 and <= 1");

	set_music_volume(volume);
	return 0;
}

