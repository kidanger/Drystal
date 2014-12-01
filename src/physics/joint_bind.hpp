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

#include "lua_util.h"

struct lua_State;
class b2Joint;

struct Joint {
	b2Joint* joint;
	Joint* nextdestroy;
	bool getting_destroyed;
	int ref;
};

typedef Joint RevoluteJoint;
typedef Joint MouseJoint;
typedef Joint DistanceJoint;
typedef Joint RopeJoint;
typedef Joint PrismaticJoint;
typedef Joint GearJoint;
typedef Joint FrictionJoint;

DECLARE_PUSH(RopeJoint, rope_joint)
DECLARE_PUSH(DistanceJoint, distance_joint)
DECLARE_PUSH(RevoluteJoint, revolute_joint)
DECLARE_PUSH(MouseJoint, mouse_joint)
DECLARE_PUSH(PrismaticJoint, prismatic_joint)
DECLARE_PUSH(GearJoint, gear_joint)
DECLARE_PUSH(FrictionJoint, friction_joint)

DECLARE_POP(Joint, joint)
Joint* pop_joint_secure(lua_State* L, int index);

int mlua_set_target_mouse_joint(lua_State* L);

int mlua_set_length_distance_joint(lua_State* L);
int mlua_set_frequency_distance_joint(lua_State* L);

int mlua_set_max_length_rope_joint(lua_State* L);

int mlua_set_angle_limits_revolute_joint(lua_State* L);
int mlua_set_motor_speed_revolute_joint(lua_State* L);

int mlua_set_enable_motor_prismatic_joint(lua_State* L);
int mlua_set_motor_speed_prismatic_joint(lua_State* L);
int mlua_set_enable_limit_prismatic_joint(lua_State* L);
int mlua_set_max_motor_force_prismatic_joint(lua_State* L);
int mlua_is_limit_enabled_prismatic_joint(lua_State* L);
int mlua_is_motor_enabled_prismatic_joint(lua_State* L);

int mlua_set_ratio_gear_joint(lua_State* L);
int mlua_get_ratio_gear_joint(lua_State* L);

int mlua_set_max_torque_friction_joint(lua_State* L);
int mlua_get_max_torque_friction_joint(lua_State* L);
int mlua_set_max_force_friction_joint(lua_State* L);
int mlua_get_max_force_friction_joint(lua_State* L);

int mlua_free_joint(lua_State* L);
