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
class b2Body;

struct Body {
	b2Body* body;
	int ref;
};

int mlua_set_active_body(lua_State* L);
int mlua_set_bullet_body(lua_State* L);
int mlua_get_mass_body(lua_State* L);
int mlua_set_mass_center_body(lua_State* L);
int mlua_apply_force_body(lua_State* L);
int mlua_apply_linear_impulse_body(lua_State* L);
int mlua_apply_angular_impulse_body(lua_State* L);
int mlua_apply_torque_body(lua_State* L);
int mlua_dump_body(lua_State* L);

#define __BODY_GET_SET(value) \
    int mlua_set_##value##_body(lua_State* L); \
    int mlua_get_##value##_body(lua_State* L);

__BODY_GET_SET(position)
__BODY_GET_SET(linear_velocity)
__BODY_GET_SET(angle)
__BODY_GET_SET(angular_velocity)
__BODY_GET_SET(linear_damping)
__BODY_GET_SET(angular_damping)
__BODY_GET_SET(linearvelocity)
__BODY_GET_SET(fixed_rotation)

#undef __BODY_GET_SET

