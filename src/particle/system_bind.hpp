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

struct lua_State;

int mlua_new_system(lua_State* L);
int mlua_set_position_system(lua_State* L);
int mlua_get_position_system(lua_State* L);
int mlua_set_offset_system(lua_State* L);
int mlua_get_offset_system(lua_State* L);
int mlua_update_system(lua_State* L);
int mlua_draw_system(lua_State* L);
int mlua_is_running_system(lua_State* L);
int mlua_set_running_system(lua_State* L);
int mlua_add_size_system(lua_State* L);
int mlua_add_color_system(lua_State* L);
int mlua_free_system(lua_State* L);

#define __SYSTEM_GET_SET(attr) \
	int mlua_get_##attr##_system(lua_State* L); \
	int mlua_set_##attr##_system(lua_State* L);

__SYSTEM_GET_SET(min_lifetime)
__SYSTEM_GET_SET(max_lifetime)
__SYSTEM_GET_SET(min_direction)
__SYSTEM_GET_SET(max_direction)
__SYSTEM_GET_SET(min_initial_acceleration)
__SYSTEM_GET_SET(max_initial_acceleration)
__SYSTEM_GET_SET(min_initial_velocity)
__SYSTEM_GET_SET(max_initial_velocity)
__SYSTEM_GET_SET(emission_rate)

#undef __SYSTEM_GET_SET

#define __ACTION(action) \
	int mlua_##action##_system(lua_State* L);
__ACTION(emit)
__ACTION(start)
__ACTION(pause)
__ACTION(stop)

#undef __ACTION

