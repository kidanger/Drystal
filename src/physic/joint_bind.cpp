#include <cassert>
#include <cstring>
#include <lua.hpp>

#include "macro.hpp"

DISABLE_WARNING_EFFCPP;
#include <Box2D/Box2D.h>
REENABLE_WARNING;

#include "joint_bind.hpp"
#include "physic_p.hpp"

inline static b2MouseJoint* luam_tomousejoint(lua_State* L, int index)
{
	return (b2MouseJoint*) luam_tojoint(L, index);
}

inline static b2DistanceJoint* luam_todistancejoint(lua_State* L, int index)
{
	return (b2DistanceJoint*) luam_tojoint(L, index);
}

inline static b2RopeJoint* luam_toropejoint(lua_State* L, int index)
{
	return (b2RopeJoint*) luam_tojoint(L, index);
}

inline static b2RevoluteJoint* luam_torevolutejoint(lua_State* L, int index)
{
	return (b2RevoluteJoint*) luam_tojoint(L, index);
}

int mlua_set_target_joint(lua_State* L)
{
	assert(L);

	b2MouseJoint* joint = luam_tomousejoint(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	joint->SetTarget(b2Vec2(x, y));
	return 0;
}

int mlua_set_length_joint(lua_State* L)
{
	assert(L);

	b2DistanceJoint* joint = luam_todistancejoint(L, 1);
	lua_Number length = luaL_checknumber(L, 2);
	joint->SetLength(length);
	return 0;
}

int mlua_set_frequency_joint(lua_State* L)
{
	assert(L);

	b2DistanceJoint* joint = luam_todistancejoint(L, 1);
	lua_Number freq = luaL_checknumber(L, 2);
	joint->SetFrequency(freq);
	return 0;
}

int mlua_set_max_length_joint(lua_State* L)
{
	assert(L);

	b2RopeJoint* joint = luam_toropejoint(L, 1);
	lua_Number maxlength = luaL_checknumber(L, 2);
	joint->SetMaxLength(maxlength);
	return 0;
}

int mlua_set_angle_limits_joint(lua_State* L)
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

int mlua_set_motor_speed_joint(lua_State* L)
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

