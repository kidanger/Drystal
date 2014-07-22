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

int mlua_new_shape(lua_State* L);
int mlua_set_sensor_shape(lua_State* L);
int mlua_gc_shape(lua_State* L);

#define __SHAPE_GET_SET(value) \
    int mlua_set_##value##_shape(lua_State* L); \
    int mlua_get_##value##_shape(lua_State* L);

__SHAPE_GET_SET(density)
__SHAPE_GET_SET(restitution)
__SHAPE_GET_SET(friction)

#undef __SHAPE_GET_SET

