#include <cassert>
#include <cstring>
#include <lua.hpp>

#include "macro.hpp"

BEGIN_DISABLE_WARNINGS;
DISABLE_WARNING_EFFCPP;
DISABLE_WARNING_STRICT_ALIASING;
#include <Box2D/Box2D.h>
END_DISABLE_WARNINGS;

#include "joint_bind.hpp"
#include "world_bind.hpp"
#include "log.hpp"
#include "lua_util.hpp"

log_category("joint");

IMPLEMENT_PUSH(RopeJoint, rope_joint)
IMPLEMENT_PUSH(DistanceJoint, distance_joint)
IMPLEMENT_PUSH(RevoluteJoint, revolute_joint)
IMPLEMENT_PUSH(MouseJoint, mouse_joint)
IMPLEMENT_PUSH(PrismaticJoint, prismatic_joint)

IMPLEMENT_POP(Joint, joint)

Joint* pop_joint_secure(lua_State* L, int index)
{
	Joint* joint = pop_joint(L, index);
	assert_lua_error(L, joint->joint, "this joint has been destroyed, it can't be used anymore");
	return joint;
}

inline static b2MouseJoint* luam_tomousejoint(lua_State* L, int index)
{
	return (b2MouseJoint *) pop_joint_secure(L, index)->joint;
}

inline static b2DistanceJoint* luam_todistancejoint(lua_State* L, int index)
{
	return (b2DistanceJoint *) pop_joint_secure(L, index)->joint;
}

inline static b2RopeJoint* luam_toropejoint(lua_State* L, int index)
{
	return (b2RopeJoint *) pop_joint_secure(L, index)->joint;
}

inline static b2RevoluteJoint* luam_torevolutejoint(lua_State* L, int index)
{
	return (b2RevoluteJoint *) pop_joint_secure(L, index)->joint;
}

inline static b2PrismaticJoint* luam_toprismaticjoint(lua_State* L, int index)
{
	return (b2PrismaticJoint *) pop_joint_secure(L, index)->joint;
}

int mlua_set_target_mouse_joint(lua_State* L)
{
	assert(L);

	b2MouseJoint* joint = luam_tomousejoint(L, 1);
	lua_Number x = luaL_checknumber(L, 2) / pixels_per_meter;
	lua_Number y = luaL_checknumber(L, 3) / pixels_per_meter;
	joint->SetTarget(b2Vec2(x, y));
	return 0;
}

int mlua_set_length_distance_joint(lua_State* L)
{
	assert(L);

	b2DistanceJoint* joint = luam_todistancejoint(L, 1);
	lua_Number length = luaL_checknumber(L, 2) / pixels_per_meter;
	joint->SetLength(length);
	return 0;
}

int mlua_set_frequency_distance_joint(lua_State* L)
{
	assert(L);

	b2DistanceJoint* joint = luam_todistancejoint(L, 1);
	lua_Number freq = luaL_checknumber(L, 2);
	joint->SetFrequency(freq);
	return 0;
}

int mlua_set_max_length_rope_joint(lua_State* L)
{
	assert(L);

	b2RopeJoint* joint = luam_toropejoint(L, 1);
	lua_Number maxlength = luaL_checknumber(L, 2) / pixels_per_meter;
	joint->SetMaxLength(maxlength);
	return 0;
}

int mlua_set_angle_limits_revolute_joint(lua_State* L)
{
	assert(L);

	b2RevoluteJoint* joint = luam_torevolutejoint(L, 1);
	lua_Number min = luaL_checknumber(L, 2);
	lua_Number max = luaL_checknumber(L, 3);
	if (min != max) {
		joint->SetLimits(min, max);
		joint->EnableLimit(true);
	} else {
		joint->EnableLimit(false);
	}
	return 0;
}

int mlua_set_motor_speed_revolute_joint(lua_State* L)
{
	assert(L);

	b2RevoluteJoint* joint = luam_torevolutejoint(L, 1);
	lua_Number speed = luaL_checknumber(L, 2);
	lua_Number maxtorque = 20;
	if (lua_gettop(L) > 3)
		maxtorque = luaL_checknumber(L, 3);
	if (speed != 0) {
		joint->SetMotorSpeed(speed);
		joint->SetMaxMotorTorque(maxtorque);
		joint->EnableMotor(true);
	} else {
		joint->EnableMotor(false);
	}
	return 0;
}

int mlua_set_enable_motor_prismatic_joint(lua_State* L)
{
	b2PrismaticJoint* joint = luam_toprismaticjoint(L, 1);
	bool enable = lua_toboolean(L, 2);
	joint->EnableMotor(enable);

	return 0;
}

int mlua_set_motor_speed_prismatic_joint(lua_State* L)
{
	b2PrismaticJoint* joint = luam_toprismaticjoint(L, 1);
	lua_Number speed = luaL_checknumber(L, 2);
	joint->SetMotorSpeed(speed);

	return 0;
}

int mlua_set_enable_limit_prismatic_joint(lua_State* L)
{
	b2PrismaticJoint* joint = luam_toprismaticjoint(L, 1);
	bool enable = lua_toboolean(L, 2);
	joint->EnableLimit(enable);

	return 0;
}

int mlua_set_max_motor_force_prismatic_joint(lua_State* L)
{
	b2PrismaticJoint* joint = luam_toprismaticjoint(L, 1);
	lua_Number force = luaL_checknumber(L, 2);
	joint->SetMaxMotorForce(force);

	return 0;
}

int mlua_is_motor_enabled_prismatic_joint(lua_State* L)
{
	b2PrismaticJoint* joint = luam_toprismaticjoint(L, 1);
	lua_pushboolean(L, joint->IsMotorEnabled());

	return 1;
}

int mlua_is_limit_enabled_prismatic_joint(lua_State* L)
{
	b2PrismaticJoint* joint = luam_toprismaticjoint(L, 1);
	lua_pushboolean(L, joint->IsLimitEnabled());

	return 1;
}

int mlua_free_joint(_unused_ lua_State* L)
{
	log_debug();
	Joint* joint = pop_joint(L, 1);
	assert_lua_error(L, !joint->joint, "joint hasn't been destroyed");
	delete joint;
	return 0;
}

