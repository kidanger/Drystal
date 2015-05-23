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
#include <assert.h>
#include <lua.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "engine.h"
#include "lua_util.h"
#include "util.h"
#include "livecoding.h"
#include "log.h"

log_category("lua");

/* from lua/src/lauxlib.c */
static int findfield(lua_State *L, int objidx, int level)
{
	assert(L);

	if (level == 0 || !lua_istable(L, -1))
		return 0;  /* not found */
	lua_pushnil(L);  /* start 'next' loop */
	while (lua_next(L, -2)) {  /* for each pair in table */
		if (lua_type(L, -2) == LUA_TSTRING) {  /* ignore non-string keys */
			if (lua_rawequal(L, objidx, -1)) {  /* found object? */
				lua_pop(L, 1);  /* remove value (but keep name) */
				return 1;
			} else if (findfield(L, objidx, level - 1)) { /* try recursively */
				lua_remove(L, -2);  /* remove table (but keep name) */
				lua_pushliteral(L, ".");
				lua_insert(L, -2);  /* place '.' between the two names */
				lua_concat(L, 3);
				return 1;
			}
		}
		lua_pop(L, 1);  /* remove value */
	}
	return 0;  /* not found */
}

static int pushglobalfuncname(lua_State *L, lua_Debug *ar)
{
	assert(L);
	assert(ar);

	int top = lua_gettop(L);
	lua_getinfo(L, "f", ar);  /* push function */
	lua_pushglobaltable(L);
	if (findfield(L, top + 1, 2)) {
		lua_copy(L, -1, top + 1);  /* move name to proper place */
		lua_pop(L, 2);  /* remove pushed values */
		return 1;
	} else {
		lua_settop(L, top);  /* remove function and global table */
		return 0;
	}
}

static void printfuncname(lua_State *L, lua_Debug *d)
{
	assert(L);
	assert(d);

	if (*d->namewhat != '\0')  /* is there a name? */
		fprintf(stderr, " in function %s%s%s" , stderr_use_colors() ? ANSI_HIGHLIGHT_PURPLE_ON : "", d->name, stderr_use_colors() ? ANSI_RESET : "");
	else if (*d->what == 'm')  /* main? */
		fprintf(stderr, " in main chunk");
	else if (*d->what == 'C') {
		if (pushglobalfuncname(L, d)) {
			fprintf(stderr, " in function %s%s%s", stderr_use_colors() ? ANSI_HIGHLIGHT_PURPLE_ON : "", lua_tostring(L, -1), stderr_use_colors() ? ANSI_RESET : "");
			lua_remove(L, -2);  /* remove name */
		} else
			fprintf(stderr, "?");
	}
}

int traceback(lua_State *L)
{
	lua_Debug d;
	int level = 0;
	int hidden_levels = 0;

	assert(L);

	// from lua/src/lua.c
	const char *msg = lua_tostring(L, 1);
	if (msg) {
		char *p;

		p = strchr(msg, ':');
		if (!p || !*(p + 1))
			log_error("%s", msg);
		else {
			p = strchr(p + 1, ':');
			if (!p || !*(p + 1) || !*(p + 2))
				log_error("%s", msg);
			else
				log_error("%s", p + 2);
		}
	} else if (!lua_isnoneornil(L, 1)) { /* is there an error object? */
		if (!luaL_callmeta(L, 1, "__tostring"))  /* try its 'tostring' metamethod */
			log_error("(no error message)");
	}

	fprintf(stderr, "stack traceback:\n");
	while (lua_getstack(L, level, &d)) {
		lua_getinfo(L, "Slnt", &d);
		if (level <= 1 && (*d.what == 'C' || startswith(d.short_src, "[string"))) {
			hidden_levels++;
			level++;
			continue;
		}
		fprintf(stderr, "\t#%d from %s%s%s", level - hidden_levels, stderr_use_colors() ? ANSI_HIGHLIGHT_ON : "", d.short_src, stderr_use_colors() ? ANSI_RESET : "");
		if (d.currentline > 0)
			fprintf(stderr, ":%s%d%s", stderr_use_colors() ? ANSI_HIGHLIGHT_GREEN_ON : "", d.currentline, stderr_use_colors() ? ANSI_RESET : "");
		printfuncname(L, &d);
		fprintf(stderr, "\n");
		if (d.istailcall)
			fprintf(stderr, "\n\t(...tail calls...)\n");
		++level;
	}

	return 0;
}


void call_lua_function(lua_State *L, int num_args, int num_ret)
{
	assert(L);
	assert(num_args >= 0);
	assert(num_ret >= 0);

#ifdef EMSCRIPTEN
	lua_call(L, num_args, num_ret);
#else
	/* from lua/src/lua.c */
	int base = lua_gettop(L) - num_args;
	lua_pushcfunction(L, traceback);
	lua_insert(L, base);
	if (lua_pcall(L, num_args, num_ret, base)) {
#ifdef BUILD_LIVECODING
		if (livecoding_is_running()) {
			engine_wait_next_reload();
			lua_pop(L, 1);
		} else
#endif
		{
			engine_free();
			exit(EXIT_FAILURE);
		}
	}
	lua_remove(L, base);
#endif
}

