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
#include <cassert>
#include <lua.hpp>

#include "macro.hpp"

BEGIN_DISABLE_WARNINGS;
DISABLE_WARNING_EFFCPP;
DISABLE_WARNING_STRICT_ALIASING;
#include <Box2D/Box2D.h>
END_DISABLE_WARNINGS;

#include "lua_functions.hpp"
#include "world_bind.hpp"
#include "body_bind.hpp"
#include "joint_bind.hpp"
#include "shape_bind.hpp"

DECLARE_PUSHPOP(Body, body)
DECLARE_PUSHPOP(RopeJoint, rope_joint)
DECLARE_PUSHPOP(DistanceJoint, distance_joint)
DECLARE_PUSHPOP(RevoluteJoint, revolute_joint)
DECLARE_PUSHPOP(MouseJoint, mouse_joint)
DECLARE_POP(Shape, shape)

static b2World* world;

int mlua_create_world(lua_State* L)
{
	assert(L);

	lua_Number gravity_x = luaL_checknumber(L, 1);
	lua_Number gravity_y = luaL_checknumber(L, 2);
	world = new b2World(b2Vec2(gravity_x, gravity_y));
	return 0;
}

int mlua_update_physic(lua_State* L)
{
	assert(L);
	if (world == NULL) {
		return luaL_error(L, "world must be created before calling update_physic");
	}

	lua_Number dt = luaL_checknumber(L, 1);

	int velocityIterations = 8;
	int positionIterations = 3;

	world->Step(dt, velocityIterations, positionIterations);
	return 0;
}

class CustomListener : public b2ContactListener
{
private:
	CustomListener(const CustomListener&);
	CustomListener& operator=(const CustomListener&);

	lua_State* L;
	int begin_contact;
	int end_contact;
	int presolve;
	int postsolve;

public:
	CustomListener(lua_State *L, int begin_contact, int end_contact, int presolve, int postsolve)
		: L(L),
		  begin_contact(begin_contact),
		  end_contact(end_contact),
		  presolve(presolve),
		  postsolve(postsolve)
	{}

	~CustomListener()
	{
		luaL_unref(L, LUA_REGISTRYINDEX, begin_contact);
		luaL_unref(L, LUA_REGISTRYINDEX, end_contact);
		luaL_unref(L, LUA_REGISTRYINDEX, presolve);
		luaL_unref(L, LUA_REGISTRYINDEX, postsolve);
	}

	void pushBodies(b2Contact* contact)
	{
		b2Body* bA = contact->GetFixtureA()->GetBody();
		b2Body* bB = contact->GetFixtureB()->GetBody();

		push_body(L, (Body*) bA->GetUserData());
		push_body(L, (Body*) bB->GetUserData());
	}

	virtual void BeginContact(b2Contact* contact)
	{
		if (begin_contact == LUA_REFNIL)
			return;
		b2WorldManifold manifold;
		contact->GetWorldManifold(&manifold);

		lua_rawgeti(L, LUA_REGISTRYINDEX, begin_contact);

		pushBodies(contact);

		lua_pushnumber(L, manifold.points[0].x);
		lua_pushnumber(L, manifold.points[0].y);
		lua_pushnumber(L, manifold.normal.x);
		lua_pushnumber(L, manifold.normal.y);

		CALL(6, 0);
	}
	virtual void EndContact(b2Contact* contact)
	{
		if (end_contact == LUA_REFNIL)
			return;

		lua_rawgeti(L, LUA_REGISTRYINDEX, end_contact);
		pushBodies(contact);

		CALL(2, 0);
	}

	virtual void PreSolve(b2Contact* contact, const b2Manifold*)
	{
		if (presolve == LUA_REFNIL)
			return;
		b2WorldManifold manifold;
		contact->GetWorldManifold(&manifold);

		lua_rawgeti(L, LUA_REGISTRYINDEX, presolve);
		pushBodies(contact);

		lua_pushnumber(L, manifold.points[0].x);
		lua_pushnumber(L, manifold.points[0].y);
		lua_pushnumber(L, manifold.normal.x);
		lua_pushnumber(L, manifold.normal.y);

		CALL(6, 1);
		bool enabled = lua_toboolean(L, -1);
		contact->SetEnabled(enabled);
	}
	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse*)
	{
		if (postsolve == LUA_REFNIL)
			return;

		lua_rawgeti(L, LUA_REGISTRYINDEX, postsolve);
		pushBodies(contact);

		CALL(2, 0);
	}
};

int mlua_on_collision(lua_State* L)
{
	assert(L);
	if (world == NULL) {
		return luaL_error(L, "world must be created before calling on_collision");
	}

	if (lua_gettop(L)) {
		int begin_contact;
		int end_contact;
		int presolve;
		int postsolve;

		lua_pushvalue(L, 1);
		begin_contact = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushvalue(L, 2);
		end_contact = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushvalue(L, 3);
		presolve = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushvalue(L, 4);
		postsolve = luaL_ref(L, LUA_REGISTRYINDEX);
		CustomListener* listener = new CustomListener(L, begin_contact, end_contact, presolve, postsolve);
		world->SetContactListener(listener);
	} else {
		CustomListener* listener = (CustomListener*) world->GetContactManager().m_contactListener;
		delete listener;
		world->SetContactListener(NULL);
	}

	return 0;
}

class CustomRayCastCallback : public b2RayCastCallback
{
private:
	CustomRayCastCallback(const CustomRayCastCallback&);
	CustomRayCastCallback& operator=(const CustomRayCastCallback&);

	lua_State* L;
	int ref;

public:
	b2Fixture* fixture;
	b2Vec2 point;

	CustomRayCastCallback(lua_State *L, int ref) : L(L), ref(ref), fixture(NULL), point(0, 0) {}

	virtual float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point,
	                              _unused_ const b2Vec2& normal, float32 fraction)
	{
		bool save_data = true;
		float32 new_fraction = fraction;

		if (ref != LUA_REFNIL) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
			Body *body = (Body *) fixture->GetBody()->GetUserData();
			push_body(L, body);
			lua_pushnumber(L, fraction);
			if (lua_pcall(L, 2, 2, 0)) {
				luaL_error(L, "error calling raycast callback: %s", lua_tostring(L, -1));
			}
			new_fraction = luaL_checknumber(L, -2);
			save_data = lua_toboolean(L, -1);
		}

		if (save_data) {
			this->fixture = fixture;
			this->point = point;
		}

		return new_fraction;
	}
};

int mlua_raycast(lua_State* L)
{
	assert(L);
	if (world == NULL) {
		return luaL_error(L, "world must be created before calling raycast");
	}

	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);
	int callback_ref = LUA_REFNIL;
	if (lua_gettop(L) == 5) {
		lua_pushvalue(L, 5);
		callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	CustomRayCastCallback callback(L, callback_ref);
	if (x1 != x2 || y1 != y2) {
		world->RayCast(&callback, b2Vec2(x1, y1), b2Vec2(x2, y2));
	}
	luaL_unref(L, LUA_REGISTRYINDEX, callback_ref);

	if (callback.fixture) {
		Body *body = (Body *) callback.fixture->GetBody()->GetUserData();
		push_body(L, body);
		lua_pushnumber(L, callback.point.x);
		lua_pushnumber(L, callback.point.y);
		return 3;
	} else {
		return 0;
	}
}

class CustomQueryCallback : public b2QueryCallback
{
private:
	CustomQueryCallback(const CustomQueryCallback&);
	CustomQueryCallback& operator=(const CustomQueryCallback&);

	lua_State* L;
	unsigned index;

public:
	CustomQueryCallback(lua_State *L) : L(L), index(1) {}

	bool ReportFixture(b2Fixture* fixture)
	{
		assert(fixture);
		Body *body = (Body *) fixture->GetBody()->GetUserData();
		lua_pushnumber(L, index);
		push_body(L, body);
		lua_settable(L, -3);
		index++;
		return true;
	}

};

int mlua_query(lua_State* L)
{
	assert(L);
	if (world == NULL) {
		return luaL_error(L, "world must be created before calling query");
	}

	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);

	lua_newtable(L);

	CustomQueryCallback query(L);
	b2AABB aabb;
	aabb.lowerBound = b2Vec2(x1, y1);
	aabb.upperBound = b2Vec2(x2, y2);

	world->QueryAABB(&query, aabb);

	return 1;
}

int mlua_new_body(lua_State* L)
{
	assert(L);
	if (world == NULL) {
		return luaL_error(L, "world must be created before calling new_body");
	}

	int index = 1;
	bool dynamic = lua_toboolean(L, index++);

	lua_Number x = 0;
	lua_Number y = 0;
	if (lua_isnumber(L, index)) { // x, y
		x = luaL_checknumber(L, index++);
		y = luaL_checknumber(L, index++);
	}

	int number_of_shapes = lua_gettop(L) - index + 1;
	b2FixtureDef* fixtureDefs[number_of_shapes];
	for (int i = 0; i < number_of_shapes; i++) {
		fixtureDefs[i] = pop_shape(L, index++)->fixtureDef;
		assert(fixtureDefs[i]);
		assert(fixtureDefs[i]->shape);
	}

	b2BodyDef def;
	if (dynamic) {
		def.type = b2_dynamicBody;
	}
	def.position.Set(x, y);

	b2Body* b2body = world->CreateBody(&def);
	for (int i = 0; i < number_of_shapes; i++) {
		b2body->CreateFixture(fixtureDefs[i]);
	}

	Body* body = new Body;
	body->body = b2body;
	body->ref = 0;
	b2body->SetUserData(body);

	push_body(L, body);
	return 1;
}

int mlua_destroy_body(lua_State* L)
{
	assert(L);
	if (world == NULL) {
		return luaL_error(L, "world must be created before calling Body:destroy");
	}

	Body* body = pop_body(L, 1);
	b2Body* b2body = body->body;
	world->DestroyBody(b2body);
	delete body;
	return 0;
}

int mlua_new_joint(lua_State* L)
{
	assert(L);
	if (world == NULL) {
		return luaL_error(L, "world must be created before calling new_joint");
	}

	b2JointDef* joint_def;

	const char * type = luaL_checkstring(L, 1);
	int i = 2;
	if (!strcmp(type, "mouse")) {
		b2MouseJointDef* def = new b2MouseJointDef;
		def->bodyA = pop_body(L, i++)->body;
		def->bodyB = pop_body(L, i++)->body;
		def->maxForce = luaL_checknumber(L, i++);
		def->target = def->bodyB->GetWorldCenter();
		joint_def = def;
	} else if (!strcmp(type, "distance")) {
		b2DistanceJointDef* def = new b2DistanceJointDef;
		b2Body* b1 = pop_body(L, i++)->body;
		b2Body* b2 = pop_body(L, i++)->body;
		def->Initialize(b1, b2, b1->GetWorldCenter(), b2->GetWorldCenter());
		joint_def = def;
	} else if (!strcmp(type, "rope")) {
		b2RopeJointDef* def = new b2RopeJointDef;
		def->bodyA = pop_body(L, i++)->body;
		def->bodyB = pop_body(L, i++)->body;
		joint_def = def;
	} else if (!strcmp(type, "revolute")) {
		b2RevoluteJointDef* def = new b2RevoluteJointDef;
		def->bodyA = pop_body(L, i++)->body;
		def->bodyB = pop_body(L, i++)->body;
		lua_Number anchorAx = luaL_checknumber(L, i++);
		lua_Number anchorAy = luaL_checknumber(L, i++);
		lua_Number anchorBx = luaL_checknumber(L, i++);
		lua_Number anchorBy = luaL_checknumber(L, i++);
		def->localAnchorA.Set(anchorAx, anchorAy);
		def->localAnchorB.Set(anchorBx, anchorBy);
		joint_def = def;
	} else {
		assert(false);
		return 0;
	}

	if (lua_gettop(L) >= i) {
		bool collide = lua_toboolean(L, i++);
		joint_def->collideConnected = collide;
	}

	assert(joint_def->bodyA);
	assert(joint_def->bodyB);

	Joint* joint = new Joint;
	joint->joint = world->CreateJoint(joint_def);
	joint->ref = 0;

	if (!strcmp(type, "mouse")) {
		push_mouse_joint(L, joint);
	} else if (!strcmp(type, "distance")) {
		push_distance_joint(L, joint);
	} else if (!strcmp(type, "rope")) {
		push_rope_joint(L, joint);
	} else if (!strcmp(type, "revolute")) {
		push_revolute_joint(L, joint);
	}

	delete joint_def;

	return 1;
}

#define __IMPLEMENT_DESTROY(T, name) \
	int mlua_destroy_##name(lua_State* L) \
	{ \
		assert(L); \
		if (world == NULL) { \
			return luaL_error(L, "world must be created before calling " #T "::destroy"); \
		} \
		T* joint = pop_##name(L, 1); \
		world->DestroyJoint(joint->joint); \
		delete joint; \
		return 0; \
	}

__IMPLEMENT_DESTROY(MouseJoint, mouse_joint)
__IMPLEMENT_DESTROY(RopeJoint, rope_joint)
__IMPLEMENT_DESTROY(DistanceJoint, distance_joint)
__IMPLEMENT_DESTROY(RevoluteJoint, revolute_joint)

#undef __IMPLEMENT_DESTROY

