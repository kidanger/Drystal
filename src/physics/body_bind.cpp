#include <cassert>
#include <lua.hpp>

#include "macro.h"

BEGIN_DISABLE_WARNINGS;
DISABLE_WARNING_EFFCPP;
DISABLE_WARNING_STRICT_ALIASING;
#include <Box2D/Box2D.h>
END_DISABLE_WARNINGS;

#include "lua_util.h"
#include "log.hpp"
#include "world_bind.hpp"
#include "body_bind.hpp"

log_category("body");

IMPLEMENT_PUSHPOP(Body, body)

Body* pop_body_secure(lua_State* L, int index)
{
	Body* body = pop_body(L, index);
	assert_lua_error(L, body->body, "this body has been destroyed, it can't be used anymore");
	return body;
}

int mlua_get_center_position_body(lua_State* L)
{
	b2Body* body = pop_body_secure(L, 1)->body;
	b2Vec2 pos = body->GetWorldCenter();
	lua_pushnumber(L, pos.x * pixels_per_meter);
	lua_pushnumber(L, pos.y * pixels_per_meter);
	return 2;
}

#define BODY_GETSET_VEC2(value, get_expr, set_expr) \
	int mlua_set_##value##_body(lua_State* L) \
	{ \
		b2Body* body = pop_body_secure(L, 1)->body; \
		lua_Number x = luaL_checknumber(L, 2) / pixels_per_meter; \
		lua_Number y = luaL_checknumber(L, 3) / pixels_per_meter; \
		b2Vec2 vector(x, y); \
		set_expr; \
		return 0; \
	} \
	int mlua_get_##value##_body(lua_State* L) \
	{ \
		b2Body* body = pop_body_secure(L, 1)->body; \
		const b2Vec2 vector = get_expr; \
		lua_pushnumber(L, vector.x * pixels_per_meter); \
		lua_pushnumber(L, vector.y * pixels_per_meter); \
		return 2; \
	}

BODY_GETSET_VEC2(position, body->GetPosition(), body->SetTransform(vector, body->GetAngle()))
BODY_GETSET_VEC2(linear_velocity, body->GetLinearVelocity(), body->SetLinearVelocity(vector))

#define BODY_GETSET_FLOAT(value, get_expr, set_expr) \
	int mlua_set_##value##_body(lua_State* L) \
	{ \
		b2Body* body = pop_body_secure(L, 1)->body; \
		lua_Number value = luaL_checknumber(L, 2); \
		set_expr; \
		return 0; \
	} \
	int mlua_get_##value##_body(lua_State* L) \
	{ \
		b2Body* body = pop_body_secure(L, 1)->body; \
		const lua_Number value = get_expr; \
		lua_pushnumber(L, value); \
		return 1; \
	}

BODY_GETSET_FLOAT(angle, body->GetAngle(), body->SetTransform(body->GetPosition(), angle))
BODY_GETSET_FLOAT(angular_velocity, body->GetAngularVelocity(), body->SetAngularVelocity(angular_velocity))
BODY_GETSET_FLOAT(linear_damping, body->GetLinearDamping(), body->SetLinearDamping(linear_damping))
BODY_GETSET_FLOAT(angular_damping, body->GetAngularDamping(), body->SetAngularDamping(angular_damping))

int mlua_set_active_body(lua_State* L)
{
	assert(L);

	b2Body* body = pop_body_secure(L, 1)->body;
	bool active = lua_toboolean(L, 2);
	body->SetActive(active);
	return 0;
}

int mlua_set_bullet_body(lua_State* L)
{
	assert(L);

	b2Body* body = pop_body_secure(L, 1)->body;
	bool bullet = lua_toboolean(L, 2);
	body->SetBullet(bullet);
	return 0;
}

int mlua_get_mass_body(lua_State* L)
{
	assert(L);

	b2Body* body = pop_body_secure(L, 1)->body;
	const lua_Number mass = body->GetMass();
	lua_pushnumber(L, mass);
	return 1;
}

int mlua_set_mass_center_body(lua_State* L)
{
	assert(L);

	b2Body* body = pop_body_secure(L, 1)->body;
	lua_Number cx = luaL_checknumber(L, 2) / pixels_per_meter;
	lua_Number cy = luaL_checknumber(L, 3) / pixels_per_meter;
	b2MassData md;
	body->GetMassData(&md);
	md.center = b2Vec2(cx, cy);
	body->SetMassData(&md);
	return 0;
}

#define BODY_GETSET_BOOL(value, get_expr, set_expr) \
	int mlua_set_##value##_body(lua_State* L) \
	{ \
		assert(L); \
		b2Body* body = pop_body_secure(L, 1)->body; \
		bool value = lua_toboolean(L, 2); \
		set_expr; \
		return 0; \
	} \
	int mlua_get_##value##_body(lua_State* L) \
	{ \
		assert(L); \
		b2Body* body = pop_body_secure(L, 1)->body; \
		const bool value = get_expr; \
		lua_pushboolean(L, value); \
		return 1; \
	}

BODY_GETSET_BOOL(fixed_rotation, body->IsFixedRotation(), body->SetFixedRotation(fixed_rotation))

int mlua_apply_force_body(lua_State* L)
{
	assert(L);

	b2Body* body = pop_body_secure(L, 1)->body;
	lua_Number fx = luaL_checknumber(L, 2) / pixels_per_meter;
	lua_Number fy = luaL_checknumber(L, 3) / pixels_per_meter;
	b2Vec2 pos;
	if (lua_gettop(L) > 4) {
		lua_Number dx = luaL_checknumber(L, 4) / pixels_per_meter;
		lua_Number dy = luaL_checknumber(L, 5) / pixels_per_meter;
		body->ApplyForce(b2Vec2(fx, fy), b2Vec2(dx, dy), true);
	} else {
		pos = body->GetWorldCenter();
		body->ApplyForceToCenter(b2Vec2(fx, fy), true);
	}
	return 0;
}

int mlua_apply_linear_impulse_body(lua_State* L)
{
	assert(L);

	b2Body* body = pop_body_secure(L, 1)->body;
	lua_Number fx = luaL_checknumber(L, 2) / pixels_per_meter;
	lua_Number fy = luaL_checknumber(L, 3) / pixels_per_meter;
	b2Vec2 pos;
	if (lua_gettop(L) > 4) {
		lua_Number dx = luaL_checknumber(L, 4) / pixels_per_meter;
		lua_Number dy = luaL_checknumber(L, 5) / pixels_per_meter;
		pos = b2Vec2(dx, dy);
	} else {
		pos = body->GetWorldCenter();
	}
	body->ApplyLinearImpulse(b2Vec2(fx, fy), pos, true);
	return 0;
}

int mlua_apply_angular_impulse_body(lua_State* L)
{
	assert(L);

	b2Body* body = pop_body_secure(L, 1)->body;
	lua_Number angle = luaL_checknumber(L, 2) / pixels_per_meter / pixels_per_meter;
	body->ApplyAngularImpulse(angle, true);
	return 0;
}

int mlua_apply_torque_body(lua_State* L)
{
	assert(L);

	b2Body* body = pop_body_secure(L, 1)->body;
	lua_Number torque = luaL_checknumber(L, 2) / pixels_per_meter / pixels_per_meter;
	body->ApplyTorque(torque, true);
	return 0;
}

int mlua_dump_body(lua_State* L)
{
	assert(L);

	b2Body* body = pop_body_secure(L, 1)->body;
	body->Dump();
	return 0;
}

int mlua_free_body(lua_State* L)
{
	log_debug();
	Body* body = pop_body(L, 1);
	assert_lua_error(L, !body->body, "body hasn't been destroyed");
	delete body;
	return 0;
}

