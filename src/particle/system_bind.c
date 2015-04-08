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
	assert(L);

	lua_Number x = luaL_checknumber(L, 1);
	lua_Number y = luaL_checknumber(L, 2);
	lua_Integer size = luaL_optinteger(L, 3, 256);

	System* system = system_new(x, y, size);

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
	system->offx = 0.f;
	system->offy = 0.f;

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
	int n = luaL_optinteger(L, 2, 1);
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
ACTION(stop)
ACTION(reset)

int mlua_draw_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	lua_Number dx = luaL_optnumber(L, 2, 0);
	lua_Number dy = luaL_optnumber(L, 3, 0);
	system_draw(system, dx, dy);
	return 0;
}

int mlua_add_size_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	lua_Number at_lifetime = luaL_checknumber(L, 2);
	lua_Number min = luaL_checknumber(L, 3);
	lua_Number max = luaL_optnumber(L, 4, min);
	system_add_size(system, at_lifetime, min, max);
	return 0;
}

int mlua_add_color_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	lua_Number at_lifetime = luaL_checknumber(L, 2);
	if (lua_gettop(L) == 5) {
		lua_Integer r = luaL_checknumber(L, 3);
		lua_Integer g = luaL_checknumber(L, 4);
		lua_Integer b = luaL_checknumber(L, 5);
		system_add_color(system, at_lifetime, r, r, g, g, b, b);
	} else {
		lua_Integer minr = luaL_checknumber(L, 3);
		lua_Integer maxr = luaL_checknumber(L, 4);
		lua_Integer ming = luaL_checknumber(L, 5);
		lua_Integer maxg = luaL_checknumber(L, 6);
		lua_Integer minb = luaL_checknumber(L, 7);
		lua_Integer maxb = luaL_checknumber(L, 8);
		system_add_color(system, at_lifetime, minr, maxr, ming, maxg, minb, maxb);
	}
	return 0;
}

int mlua_clear_sizes_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	system_clear_sizes(system);
	return 0;
}

int mlua_clear_colors_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	system_clear_colors(system);
	return 0;
}

int mlua_set_texture_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	Surface* surface = pop_surface(L, 2);
	system_set_texture(system, surface);
	return 0;
}

int mlua_clone_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	push_system(L, system_clone(system));
	return 1;
}

int mlua_free_system(lua_State* L)
{
	assert(L);

	System* system = pop_system(L, 1);
	system_free(system);
	return 0;
}

