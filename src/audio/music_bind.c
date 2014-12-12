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
#include <lua.h>
#include <lauxlib.h>

#include "log.h"
#include "music_bind.h"
#include "music.h"
#include "lua_util.h"
#include "util.h"

log_category("music");

IMPLEMENT_PUSHPOP(Music, music)

typedef struct LuaMusicCallback LuaMusicCallback;
struct LuaMusicCallback {
	MusicCallback base;

	lua_State* L;
	int ref;
	int table_ref;
};

static unsigned int lmc_feed_buffer(MusicCallback *mc, unsigned short *buffer, unsigned int len)
{
	LuaMusicCallback *lmc = (LuaMusicCallback *) mc;
	lua_State *L;
	unsigned int k;
	unsigned int i;

	assert(lmc);
	assert(buffer);

	L = lmc->L;

	if (lmc->table_ref == LUA_NOREF) {
		lua_createtable(L, len, 0);
		lmc->table_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}
	lua_rawgeti(L, LUA_REGISTRYINDEX, lmc->ref);
	lua_rawgeti(L, LUA_REGISTRYINDEX, lmc->table_ref);
	lua_pushunsigned(L, len);
	lua_call(L, 2, 1);

	i = lua_tounsigned(L, -1);
	lua_pop(L, 1);

	lua_rawgeti(L, LUA_REGISTRYINDEX, lmc->table_ref);
	for (k = 1; k <= i; k++) {
		lua_rawgeti(L, -1, k);
		lua_Number sample = luaL_checknumber(L, -1);
		buffer[k] = sample * (1 << 15) + (1 << 15);
		lua_pop(L, 1);
	}

	return i;
}

static void lmc_rewind(_unused_ MusicCallback *mc)
{
}

static void lmc_free(MusicCallback *mc)
{
	LuaMusicCallback *lmc = (LuaMusicCallback *) mc;

	if (!lmc)
		return;

	luaL_unref(lmc->L, LUA_REGISTRYINDEX, lmc->ref);
	if (lmc->table_ref != LUA_NOREF)
		luaL_unref(lmc->L, LUA_REGISTRYINDEX, lmc->table_ref);

	free(lmc);
}

static LuaMusicCallback *lmc_new(lua_State *L)
{
	LuaMusicCallback *lmc;

	lmc = new(LuaMusicCallback, 1);

	lmc->L = L;
	lua_pushvalue(L, 1);
	lmc->ref = luaL_ref(L, LUA_REGISTRYINDEX);
	lmc->table_ref = LUA_NOREF;
	lmc->base.free = lmc_free;
	lmc->base.rewind = lmc_rewind;
	lmc->base.feed_buffer = lmc_feed_buffer;

	return lmc;
}

int mlua_load_music(lua_State *L)
{
	Music *music = NULL;

	assert(L);

	if (lua_isstring(L, 1)) {
		const char* filename = lua_tostring(L, 1);
		music = music_load_from_file(filename);
		if (!music) {
			return luaL_fileresult(L, 0, filename);
		}
	} else {
		LuaMusicCallback *callback = lmc_new(L);

		int samplesrate = luaL_optint(L, 2, DEFAULT_SAMPLES_RATE);
		music = music_load((MusicCallback *) callback, samplesrate, 1);
	}

	push_music(L, music);
	return 1;
}

int mlua_play_music(lua_State *L)
{
	assert(L);

	Music* music = pop_music(L, 1);
	bool loop = lua_toboolean(L, 2);
	music_play(music, loop);
	return 0;
}

int mlua_stop_music(lua_State* L)
{
	assert(L);

	Music* music = pop_music(L, 1);
	music_stop(music);
	return 0;
}

int mlua_pause_music(lua_State* L)
{
	assert(L);

	Music* music = pop_music(L, 1);
	music_pause(music);
	return 0;
}


int mlua_free_music(lua_State *L)
{
	assert(L);

	log_debug("");
	Music* music = pop_music(L, 1);
	music_free(music);
	return 0;
}

