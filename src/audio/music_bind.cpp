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
#include "music_bind.hpp"
#include "music.hpp"
#include "lua_util.hpp"

log_category("music");

IMPLEMENT_PUSHPOP(Music, music)

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

	void rewind()
	{
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
		music = Music::load_from_file(filename);
		if (!music) {
			return luaL_fileresult(L, 0, filename);
		}
	} else {
		LuaMusicCallback* callback = new LuaMusicCallback;
		callback->L = L;
		lua_pushvalue(L, 1);
		callback->ref = luaL_ref(L, LUA_REGISTRYINDEX);

		int samplesrate = luaL_optint(L, 2, DEFAULT_SAMPLES_RATE);
		music = Music::load(callback, samplesrate);
	}
	push_music(L, music);
	return 1;
}

int mlua_play_music(lua_State *L)
{
	assert(L);

	Music* music = pop_music(L, 1);
	music->play();
	return 0;
}

int mlua_stop_music(lua_State* L)
{
	assert(L);

	Music* music = pop_music(L, 1);
	music->stop();
	return 0;
}

int mlua_free_music(lua_State *L)
{
	assert(L);

	log_debug("");
	Music* music = pop_music(L, 1);
	music->free();
	return 0;
}

