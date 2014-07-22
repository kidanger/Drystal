#include <cassert>
#include <lua.hpp>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Weffc++"
#include <Box2D/Box2D.h>
#pragma GCC diagnostic pop

#include "body_bind.hpp"
#include "physic_p.hpp"

#define BODY_GETSET_VEC2(value, get_expr, set_expr) \
	int mlua_set_##value##_body(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
		lua_Number x = luaL_checknumber(L, 2); \
		lua_Number y = luaL_checknumber(L, 3); \
		b2Vec2 vector(x, y); \
		set_expr; \
		return 0; \
	} \
	int mlua_get_##value##_body(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
		const b2Vec2 vector = get_expr; \
		lua_pushnumber(L, vector.x); \
		lua_pushnumber(L, vector.y); \
		return 2; \
	}

BODY_GETSET_VEC2(position, body->GetPosition(), body->SetTransform(vector, body->GetAngle()))
BODY_GETSET_VEC2(linear_velocity, body->GetLinearVelocity(), body->SetLinearVelocity(vector))

#define BODY_GETSET_FLOAT(value, get_expr, set_expr) \
	int mlua_set_##value##_body(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
		lua_Number value = luaL_checknumber(L, 2); \
		set_expr; \
		return 0; \
	} \
	int mlua_get_##value##_body(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
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

	b2Body* body = luam_tobody(L, 1);
	bool active = lua_toboolean(L, 2);
	body->SetActive(active);
	return 0;
}

int mlua_set_bullet_body(lua_State* L)
{
	assert(L);

	b2Body* body = luam_tobody(L, 1);
	bool bullet = lua_toboolean(L, 2);
	body->SetBullet(bullet);
	return 0;
}

int mlua_get_mass_body(lua_State* L)
{
	assert(L);

	b2Body* body = luam_tobody(L, 1);
	const lua_Number mass = body->GetMass();
	lua_pushnumber(L, mass);
	return 1;
}

int mlua_set_mass_center_body(lua_State* L)
{
	assert(L);

	b2Body* body = luam_tobody(L, 1);
	lua_Number cx = luaL_checknumber(L, 2);
	lua_Number cy = luaL_checknumber(L, 3);
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
		b2Body* body = luam_tobody(L, 1); \
		bool value = lua_toboolean(L, 2); \
		set_expr; \
		return 0; \
	} \
	int mlua_get_##value##_body(lua_State* L) \
	{ \
		assert(L); \
		b2Body* body = luam_tobody(L, 1); \
		const bool value = get_expr; \
		lua_pushboolean(L, value); \
		return 1; \
	}

BODY_GETSET_BOOL(fixed_rotation, body->IsFixedRotation(), body->SetFixedRotation(fixed_rotation))

int mlua_apply_force_body(lua_State* L)
{
	assert(L);

	b2Body* body = luam_tobody(L, 1);
	lua_Number fx = luaL_checknumber(L, 2);
	lua_Number fy = luaL_checknumber(L, 3);
	b2Vec2 pos;
	if (lua_gettop(L) > 4) {
		lua_Number dx = luaL_checknumber(L, 4);
		lua_Number dy = luaL_checknumber(L, 5);
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

	b2Body* body = luam_tobody(L, 1);
	lua_Number fx = luaL_checknumber(L, 2);
	lua_Number fy = luaL_checknumber(L, 3);
	b2Vec2 pos;
	if (lua_gettop(L) > 4) {
		lua_Number dx = luaL_checknumber(L, 4);
		lua_Number dy = luaL_checknumber(L, 5);
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

	b2Body* body = luam_tobody(L, 1);
	lua_Number angle = luaL_checknumber(L, 2);
	body->ApplyAngularImpulse(angle, true);
	return 0;
}

int mlua_apply_torque_body(lua_State* L)
{
	assert(L);

	b2Body* body = luam_tobody(L, 1);
	lua_Number torque = luaL_checknumber(L, 2);
	body->ApplyTorque(torque, true);
	return 0;
}

int mlua_dump_body(lua_State* L)
{
	assert(L);

	b2Body* body = luam_tobody(L, 1);
	body->Dump();
	return 0;
}

