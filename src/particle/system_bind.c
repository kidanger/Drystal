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

#include <math.h>
#include <lua.h>
#include <lauxlib.h>

#include "system.h"
#include "system_bind.h"
#include "lua_util.h"
#include "graphics/display_bind.h" // pop_surface

IMPLEMENT_PUSHPOP(System, system)

int mlua_new_system(lua_State* L)
{
	int x;
	int y;

	assert(L);

	x = luaL_checknumber(L, 1);
	y = luaL_checknumber(L, 2);

	System* system = system_new(x, y);

	system->min_direction = 0;
	system->max_direction = M_PI * 2;

	system->sizes[0].at = 0;
	system->sizes[0].min = 5;
	system->sizes[0].max = 5;
	system->sizes[1].at = 1;
	system->sizes[1].min = 5;
	system->sizes[1].max = 5;
	system->min_lifetime = 3;
	system->max_lifetime = 10;

	system->min_initial_acceleration = RAND(-10, 10);
	system->max_initial_acceleration = system->min_initial_acceleration + 3;
	system->min_initial_velocity = RAND(10, 100);
	system->max_initial_velocity = system->min_initial_velocity + RAND(10, 100);

	system->colors[0].at = 0;
	system->colors[0].min_r = RAND(0, 125);
	system->colors[0].max_r = system->colors[0].min_r + RAND(0, 50);
	system->colors[0].min_g = RAND(0, 125);
	system->colors[0].max_g = system->colors[0].min_g + RAND(0, 50);
	system->colors[0].min_b = RAND(0, 125);
	system->colors[0].max_b = system->colors[0].min_b + RAND(0, 50);
	system->colors[1].at = 1;
	system->colors[1].min_r = RAND(0, 125);
	system->colors[1].max_r = system->colors[0].min_r + RAND(0, 50);
	system->colors[1].min_g = RAND(0, 125);
	system->colors[1].max_g = system->colors[0].min_g + RAND(0, 50);
	system->colors[1].min_b = RAND(0, 125);
	system->colors[1].max_b = system->colors[0].min_b + RAND(0, 50);

	system->emission_rate = RAND(1, 19);
	system->offx = 0;
	system->offy = 0;

	push_system(L, system);
	return 1;
}

int mlua_set_position_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	system->x = x;
	system->y = y;
	return 0;
}

int mlua_get_position_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	lua_pushnumber(L, system->x);
	lua_pushnumber(L, system->y);
	return 2;
}

int mlua_set_offset_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	lua_Number ox = luaL_checknumber(L, 2);
	lua_Number oy = luaL_checknumber(L, 3);
	system->offx = ox;
	system->offy = oy;
	return 0;
}

int mlua_get_offset_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	lua_pushnumber(L, system->offx);
	lua_pushnumber(L, system->offy);
	return 2;
}

#define GETSET(attr) \
	int mlua_get_##attr##_system(lua_State* L) \
	{ \
		assert(L); \
		System* system = pop_system(L, 1);\
		lua_pushnumber(L, system->attr); \
		return 1; \
	} \
	int mlua_set_##attr##_system(lua_State* L) \
	{ \
		assert(L); \
		System* system = pop_system(L, 1);\
		lua_Number attr = luaL_checknumber(L, 2); \
		system->attr = attr; \
		return 0; \
	}

GETSET(min_lifetime)
GETSET(max_lifetime)
GETSET(min_direction)
GETSET(max_direction)
GETSET(min_initial_acceleration)
GETSET(max_initial_acceleration)
GETSET(min_initial_velocity)
GETSET(max_initial_velocity)
GETSET(emission_rate)

int mlua_update_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	lua_Number dt = luaL_checknumber(L, 2);
	system_update(system, dt);
	return 0;
}

int mlua_emit_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	int n = luaL_optint(L, 2, 1);
	while (n--)
		system_emit(system);
	return 0;
}

#define ACTION(action) \
	int mlua_##action##_system(lua_State* L) \
	{ \
		assert(L); \
		System* system = pop_system(L, 1);\
		system_##action(system); \
		return 0; \
	}

ACTION(start)
ACTION(pause)
ACTION(stop)

int mlua_draw_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	lua_Number dx = 0;
	lua_Number dy = 0;
	if (lua_gettop(L) > 1) {
		dx = luaL_checknumber(L, 2);
		dy = luaL_checknumber(L, 3);
	}
	system_draw(system, dx, dy);
	return 0;
}

int mlua_is_running_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	bool running = system->running;
	lua_pushboolean(L, running);
	return 1;
}

int mlua_set_running_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	bool running = lua_toboolean(L, 2);
	system->running = running;
	return 0;
}

int mlua_add_size_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	lua_Number at_lifetime = luaL_checknumber(L, 2);
	lua_Number min = luaL_checknumber(L, 3);
	lua_Number max = min;
	if (lua_gettop(L) > 3) {
		max = luaL_checknumber(L, 4);
	}
	system_add_size(system, at_lifetime, min, max);
	return 0;
}

int mlua_add_color_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	lua_Number at_lifetime = luaL_checknumber(L, 2);
	if (lua_gettop(L) == 5) {
		lua_Number r = luaL_checknumber(L, 3);
		lua_Number g = luaL_checknumber(L, 4);
		lua_Number b = luaL_checknumber(L, 5);
		system_add_color(system, at_lifetime, r, r, g, g, b, b);
	} else {
		lua_Number minr = luaL_checknumber(L, 3);
		lua_Number maxr = luaL_checknumber(L, 4);
		lua_Number ming = luaL_checknumber(L, 5);
		lua_Number maxg = luaL_checknumber(L, 6);
		lua_Number minb = luaL_checknumber(L, 7);
		lua_Number maxb = luaL_checknumber(L, 8);
		system_add_color(system, at_lifetime, minr, maxr, ming, maxg, minb, maxb);
	}
	return 0;
}

int mlua_set_texture_system(lua_State* L)
{
	System* system = pop_system(L, 1);
	Surface* surface = pop_surface(L, 2);
	system_set_texture(system, surface);
	return 0;
}

int mlua_free_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	system_free(system);
	return 0;
}

