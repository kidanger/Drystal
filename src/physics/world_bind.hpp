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

extern float pixels_per_meter;

int mlua_init_physics(lua_State* L);
int mlua_set_gravity(lua_State* L);
int mlua_get_gravity(lua_State* L);
int mlua_set_pixels_per_meter(lua_State* L);
int mlua_get_pixels_per_meter(lua_State* L);
int mlua_update_physics(lua_State* L);
int mlua_on_collision(lua_State* L);
int mlua_raycast(lua_State* L);
int mlua_query(lua_State* L);
int mlua_new_body(lua_State* L);
int mlua_destroy_body(lua_State* L);
int mlua_new_joint(lua_State* L);
int mlua_destroy_joint(lua_State* L);

