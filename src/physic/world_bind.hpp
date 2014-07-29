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

int mlua_create_world(lua_State* L);
int mlua_update_physic(lua_State* L);
int mlua_on_collision(lua_State* L);
int mlua_raycast(lua_State* L);
int mlua_query(lua_State* L);
int mlua_new_body(lua_State* L);
int mlua_destroy_body(lua_State* L);
int mlua_new_joint(lua_State* L);
#define __DECLARE_DESTROY(T, name) \
	int mlua_destroy_##name(lua_State* L);
__DECLARE_DESTROY(MouseJoint, mouse_joint)
__DECLARE_DESTROY(RopeJoint, rope_joint)
__DECLARE_DESTROY(DistanceJoint, distance_joint)
__DECLARE_DESTROY(RevoluteJoint, revolute_joint)
__DECLARE_DESTROY(PrismaticJoint, prismatic_joint)
#undef __DECLARE_DESTROY

