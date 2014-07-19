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
#include "audiomanager.hpp"
#include "log.hpp"
#include "api.hpp"

static AudioManager& audio = get_audiomanager();

DECLARE_PUSHPOP(Sound, sound)
DECLARE_PUSHPOP(Music, music)

int mlua_load_sound(lua_State *L)
{
	assert(L);

	const char* filename = lua_tostring(L, 1);
	Sound *chunk = audio.load_sound(filename);
	if (chunk) {
		push_sound(L, chunk);
		return 1;
	}
	return luaL_fileresult(L, 0, filename);
}

int mlua_create_sound(lua_State *L)
{
	assert(L);

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

	Sound *chunk = audio.create_sound(len, buffer);
	push_sound(L, chunk);
	return 1;
}

int mlua_play_sound(lua_State *L)
{
	assert(L);

	Sound* chunk = pop_sound(L, 1);

	float volume = 1;
	float x = 0;
	float y = 0;
	if (!lua_isnone(L, 2))
		volume = luaL_checknumber(L, 2);
	if (!lua_isnone(L, 3))
		x = luaL_checknumber(L, 3);
	if (!lua_isnone(L, 4))
		y = luaL_checknumber(L, 4);

	audio.play_sound(chunk, volume, x, y);
	return 0;
}

int mlua_free_sound(lua_State *L)
{
	assert(L);

	DEBUG("");
	Sound* chunk = pop_sound(L, 1);
	audio.free_sound(chunk);
	return 0;
}

int mlua_free_music(lua_State *L)
{
	assert(L);

	DEBUG("");
	Music* music = pop_music(L, 1);
	audio.free_music(music);
	return 0;
}

class LuaMusicCallback : public MusicCallback
{
public:
	lua_State* L;
	int ref;
	int table_ref;

	LuaMusicCallback() :
		L(NULL),
		ref(LUA_NOREF),
		table_ref(LUA_NOREF)
	{
	}

	unsigned int feed_buffer(unsigned short* buffer, unsigned int len)
	{
		if (table_ref == LUA_NOREF) {
			lua_createtable(L, len, 0);
			this->table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
		}
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
		lua_rawgeti(L, LUA_REGISTRYINDEX, this->table_ref);
		lua_pushunsigned(L, len);
		lua_call(L, 2, 1);

		unsigned int i = lua_tounsigned(L, -1);
		lua_pop(L, 1);

		lua_rawgeti(L, LUA_REGISTRYINDEX, table_ref);
		for (unsigned int k = 1; k <= i; k++) {
			lua_rawgeti(L, -1, k);
			lua_Number sample = luaL_checknumber(L, -1);
			buffer[k] = sample * (1 << 15) + (1 << 15);
			lua_pop(L, 1);
		}
		return i;
	}

	~LuaMusicCallback()
	{
		luaL_unref(L, LUA_REGISTRYINDEX, ref);
		if (table_ref != LUA_NOREF)
			luaL_unref(L, LUA_REGISTRYINDEX, table_ref);
	}
private:
	LuaMusicCallback(const LuaMusicCallback&);
	LuaMusicCallback& operator=(const LuaMusicCallback&);
};

int mlua_load_music(lua_State *L)
{
	assert(L);

	Music* music;
	if (lua_isstring(L, 1)) {
		const char* filename = lua_tostring(L, 1);
		music = audio.load_music_from_file(filename);
		if (!music) {
			return luaL_fileresult(L, 0, filename);
		}
	} else {
		LuaMusicCallback* callback = new LuaMusicCallback;
		callback->L = L;
		lua_pushvalue(L, 1);
		callback->ref = luaL_ref(L, LUA_REGISTRYINDEX);

		int samplesrate = luaL_optnumber(L, 2, DEFAULT_SAMPLES_RATE);
		music = audio.load_music(callback, samplesrate);
	}
	push_music(L, music);
	return 1;
}

int mlua_play_music(lua_State *L)
{
	assert(L);

	Music* music = pop_music(L, 1);
	audio.play_music(music);
	return 0;
}

int mlua_set_sound_volume(lua_State *L)
{
	assert(L);

	float volume = luaL_checknumber(L, 1);
	audio.set_sound_volume(volume);
	return 0;
}

int mlua_set_music_volume(lua_State *L)
{
	assert(L);

	float volume = luaL_checknumber(L, 1);
	audio.set_music_volume(volume);
	return 0;
}

int mlua_stop_music(lua_State* L)
{
	assert(L);

	Music* music = pop_music(L, 1);
	audio.stop_music(music);
	return 0;
}

