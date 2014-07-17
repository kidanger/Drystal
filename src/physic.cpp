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
#include <lua.hpp>

#include "engine.hpp"

#include <Box2D/Box2D.h>

static b2World* world;

int mlua_create_world(lua_State* L)
{
	lua_Number gravity_x = luaL_checknumber(L, 1);
	lua_Number gravity_y = luaL_checknumber(L, 2);
	world = new b2World(b2Vec2(gravity_x, gravity_y));
	return 0;
}

int mlua_update_physic(lua_State* L)
{
	assert(world);
	lua_Number dt = luaL_checknumber(L, 1);

	int velocityIterations = 8;
	int positionIterations = 3;

	world->Step(dt, velocityIterations, positionIterations);
	return 0;
}

class CustomListener : public b2ContactListener
{
public:
	int begin_contact;
	int end_contact;
	int presolve;
	int postsolve;
	lua_State* L;

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

		int refA = (int) (long) bA->GetUserData();
		lua_rawgeti(L, LUA_REGISTRYINDEX, refA);
		int refB = (int) (long) bB->GetUserData();
		lua_rawgeti(L, LUA_REGISTRYINDEX, refB);
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
	assert(world);

	if (lua_gettop(L)) {
		CustomListener* listener = new CustomListener;
		listener->L = L;
		lua_pushvalue(L, 1);
		listener->begin_contact = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushvalue(L, 2);
		listener->end_contact = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushvalue(L, 3);
		listener->presolve = luaL_ref(L, LUA_REGISTRYINDEX);
		lua_pushvalue(L, 4);
		listener->postsolve = luaL_ref(L, LUA_REGISTRYINDEX);
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
public:
	lua_State* L;
	int ref;

	b2Fixture* fixture;
	b2Vec2 point;

	CustomRayCastCallback() : fixture(NULL)
	{}

	virtual float32 ReportFixture(b2Fixture* fixture, const b2Vec2& point,
								  const b2Vec2& normal, float32 fraction)
	{
		(void) normal;
		bool save_data = true;
		float32 new_fraction = fraction;

		if (ref != LUA_REFNIL) {
			lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
			int refbody = (int) (size_t) fixture->GetBody()->GetUserData();
			lua_rawgeti(L, LUA_REGISTRYINDEX, refbody);
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
	assert(world);

	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);
	int callback_ref = LUA_REFNIL;
	if (lua_gettop(L) == 5) {
		lua_pushvalue(L, 5);
		callback_ref = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	CustomRayCastCallback callback;
	callback.L = L;
	callback.ref = callback_ref;
	if (x1 != x2 || y1 != y2) {
		world->RayCast(&callback, b2Vec2(x1, y1), b2Vec2(x2, y2));
	}
	luaL_unref(L, LUA_REGISTRYINDEX, callback_ref);

	if (callback.fixture) {
		int ref = (int) (size_t) callback.fixture->GetBody()->GetUserData();
		lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
		lua_pushnumber(L, callback.point.x);
		lua_pushnumber(L, callback.point.y);
		return 3;
	} else {
		return 0;
	}
}

class CustomQueryCallback : public b2QueryCallback
{
	public:
		lua_State* L;
		unsigned index;

		CustomQueryCallback() : index(1)
		{}

		bool ReportFixture(b2Fixture* fixture)
		{
			int ref = (int) (size_t) fixture->GetBody()->GetUserData();
			lua_pushnumber(L, index);
			lua_rawgeti(L, LUA_REGISTRYINDEX, ref);
			lua_settable(L, -3);
			index++;
			return true;
		}

};

int mlua_query(lua_State* L)
{
	assert(world);

	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);

	lua_newtable(L);

	CustomQueryCallback query;
	query.L = L;
	b2AABB aabb;
	aabb.lowerBound = b2Vec2(x1, y1);
	aabb.upperBound = b2Vec2(x2, y2);

	world->QueryAABB(&query, aabb);

	return 1;
}

// Shape methods

int mlua_new_shape(lua_State* L)
{
	assert(world);

	const char* type = luaL_checkstring(L, 1);

	b2FixtureDef* fixtureDef = new b2FixtureDef;
	fixtureDef->density = 1.0f;

	if (!strcmp(type, "box")) {
		b2PolygonShape* polygon = new b2PolygonShape;
		lua_Number w = luaL_checknumber(L, 2) / 2;
		lua_Number h = luaL_checknumber(L, 3) / 2;
		lua_Number centerx = 0;
		lua_Number centery = 0;
		if (lua_gettop(L) > 3) {
			centerx = luaL_checknumber(L, 4);
			centery = luaL_checknumber(L, 5);
		}
		polygon->SetAsBox(w, h, b2Vec2(centerx, centery), 0);
		fixtureDef->shape = polygon;
	} else if (!strcmp(type, "circle")) {
		b2CircleShape* circle = new b2CircleShape;
		circle->m_radius = luaL_checknumber(L, 2);
		if (lua_gettop(L) > 2) {
			lua_Number dx = luaL_checknumber(L, 3);
			lua_Number dy = luaL_checknumber(L, 4);
			circle->m_p.Set(dx, dy);
		}
		fixtureDef->shape = circle;
	} else if (!strcmp(type, "chain")) {
		b2ChainShape* chain = new b2ChainShape;
		int number = (lua_gettop(L) - 1) / 2;
		b2Vec2* vecs = new b2Vec2[number];
		for (int i = 0; i < number; i++) {
			vecs[i].x = luaL_checknumber(L, (i + 1)*2);
			vecs[i].y = luaL_checknumber(L, (i + 1)*2 + 1);
		}
		chain->CreateLoop(vecs, number);
		delete[] vecs;
		fixtureDef->shape = chain;
	} else {
		assert(false);
		return 0;
	}

	lua_newtable(L);
	lua_pushlightuserdata(L, fixtureDef);
	lua_setfield(L, -2, "__self");
	luaL_getmetatable(L, "shape");
	lua_setmetatable(L, -2);
	return 1;
}

static b2FixtureDef* luam_tofixture(lua_State* L, int index)
{
	luaL_checktype(L, index, LUA_TTABLE);
	lua_getfield(L, index, "__self");
	b2FixtureDef* shape = (b2FixtureDef*) lua_touserdata(L, -1);
	return shape;
}

#define SHAPE_GETSET_SOME_VALUE(value) \
	int mlua_set_##value##_shape(lua_State* L) \
	{ \
		b2FixtureDef* fixtureDef = luam_tofixture(L, 1); \
		lua_Number value = luaL_checknumber(L, 2); \
		fixtureDef->value = value; \
		return 0; \
	} \
	int mlua_get_##value##_shape(lua_State* L) \
	{ \
		b2FixtureDef* fixtureDef = luam_tofixture(L, 1); \
		lua_pushnumber(L, fixtureDef->value); \
		return 1; \
	}
SHAPE_GETSET_SOME_VALUE(density)
SHAPE_GETSET_SOME_VALUE(restitution)
SHAPE_GETSET_SOME_VALUE(friction)

int mlua_set_sensor_shape(lua_State* L)
{
	b2FixtureDef* fixtureDef = luam_tofixture(L, 1);
	bool sensor = lua_toboolean(L, 2);
	fixtureDef->isSensor = sensor;
	return 0;
}

int mlua_gc_shape(lua_State* L)
{
	b2FixtureDef* fixtureDef = luam_tofixture(L, 1);
	delete fixtureDef->shape;
	delete fixtureDef;
	return 0;
}

#define DECLARE_SHAPE_FUNCTION(name) {#name, shape_##name}
#define DECLARE_SHAPE_GETSET(x) DECLARE_SHAPE_FUNCTION(set_##x), DECLARE_SHAPE_FUNCTION(get_##x)

// Body methods

int mlua_new_body(lua_State* L)
{
	assert(world);

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
		fixtureDefs[i] = luam_tofixture(L, index++);
		assert(fixtureDefs[i]);
		assert(fixtureDefs[i]->shape);
	}

	b2BodyDef def;
	if (dynamic) {
		def.type = b2_dynamicBody;
	}
	def.position.Set(x, y);

	b2Body* body = world->CreateBody(&def);
	for (int i = 0; i < number_of_shapes; i++) {
		body->CreateFixture(fixtureDefs[i]);
	}

	lua_newtable(L);
	lua_pushlightuserdata(L, body);
	lua_setfield(L, -2, "__self");
	luaL_getmetatable(L, "body");
	lua_setmetatable(L, -2);

	lua_pushvalue(L, -1);
	body->SetUserData((void*) (size_t) luaL_ref(L, LUA_REGISTRYINDEX));
	return 1;
}

static b2Body* luam_tobody(lua_State* L, int index)
{
	luaL_checktype(L, index, LUA_TTABLE);
	lua_getfield(L, index, "__self");
	b2Body* body = (b2Body*) lua_touserdata(L, -1);
	return body;
}

#define BODY_GETSET_VEC2(value, get_expr, set_expr) \
	static int mlua_set_##value##_body(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
		lua_Number x = luaL_checknumber(L, 2); \
		lua_Number y = luaL_checknumber(L, 3); \
		b2Vec2 vector(x, y); \
		set_expr; \
		return 0; \
	} \
	static int mlua_get_##value##_body(lua_State* L) \
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
	static int mlua_set_##value##_body(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
		lua_Number value = luaL_checknumber(L, 2); \
		set_expr; \
		return 0; \
	} \
	static int mlua_get_##value##_body(lua_State* L) \
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

static int mlua_set_active_body(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	bool active = lua_toboolean(L, 2);
	body->SetActive(active);
	return 0;
}

static int mlua_set_bullet_body(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	bool bullet = lua_toboolean(L, 2);
	body->SetBullet(bullet);
	return 0;
}

static int mlua_get_mass_body(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	const lua_Number mass = body->GetMass();
	lua_pushnumber(L, mass);
	return 1;
}

static int mlua_set_mass_center_body(lua_State* L)
{
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
	static int mlua_set_##value##_body(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
		bool value = lua_toboolean(L, 2); \
		set_expr; \
		return 0; \
	} \
	static int mlua_get_##value##_body(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
		const bool value = get_expr; \
		lua_pushboolean(L, value); \
		return 1; \
	}

BODY_GETSET_BOOL(fixed_rotation, body->IsFixedRotation(), body->SetFixedRotation(fixed_rotation))

static int mlua_apply_force_body(lua_State* L)
{
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
static int mlua_apply_linear_impulse_body(lua_State* L)
{
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
static int mlua_apply_angular_impulse_body(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	lua_Number angle = luaL_checknumber(L, 2);
	body->ApplyAngularImpulse(angle, true);
	return 0;
}
static int mlua_apply_torque_body(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	lua_Number torque = luaL_checknumber(L, 2);
	body->ApplyTorque(torque, true);
	return 0;
}

static int mlua_dump_body(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	body->Dump();
	return 0;
}

static int mlua_destroy_body(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	world->DestroyBody(body);
	return 0;
}

// Joint methods

int mlua_new_joint(lua_State* L)
{
	assert(world);

	b2JointDef* joint_def;

	const char * type = luaL_checkstring(L, 1);
	int i = 2;
	if (!strcmp(type, "mouse")) {
		b2MouseJointDef* def = new b2MouseJointDef;
		def->bodyA = luam_tobody(L, i++);
		def->bodyB = luam_tobody(L, i++);
		def->maxForce = luaL_checknumber(L, i++);
		def->target = def->bodyB->GetWorldCenter();
		joint_def = def;
	} else if (!strcmp(type, "distance")) {
		b2DistanceJointDef* def = new b2DistanceJointDef;
		b2Body* b1 = luam_tobody(L, i++);
		b2Body* b2 = luam_tobody(L, i++);
		def->Initialize(b1, b2, b1->GetWorldCenter(), b2->GetWorldCenter());
		joint_def = def;
	} else if (!strcmp(type, "rope")) {
		b2RopeJointDef* def = new b2RopeJointDef;
		def->bodyA = luam_tobody(L, i++);
		def->bodyB = luam_tobody(L, i++);
		joint_def = def;
	} else if (!strcmp(type, "revolute")) {
		b2RevoluteJointDef* def = new b2RevoluteJointDef;
		def->bodyA = luam_tobody(L, i++);
		def->bodyB = luam_tobody(L, i++);
		lua_Number anchorAx = luaL_checknumber(L, i++);
		lua_Number anchorAy = luaL_checknumber(L, i++);
		lua_Number anchorBx = luaL_checknumber(L, i++);
		lua_Number anchorBy = luaL_checknumber(L, i++);
		def->localAnchorA.Set(anchorAx, anchorAy);
		def->localAnchorB.Set(anchorBx, anchorBy);
		joint_def = def;
	} else {
		assert(false);
	}

	if (lua_gettop(L) >= i) {
		bool collide = lua_toboolean(L, i++);
		joint_def->collideConnected = collide;
	}

	assert(joint_def->bodyA);
	assert(joint_def->bodyB);
	b2Joint* joint = world->CreateJoint(joint_def);

	delete joint_def;

	lua_newtable(L);
	lua_pushlightuserdata(L, joint);
	lua_setfield(L, -2, "__self");
	luaL_getmetatable(L, "joint");
	lua_setmetatable(L, -2);
	return 1;
}

static b2Joint* luam_tojoint(lua_State* L, int index)
{
	luaL_checktype(L, index, LUA_TTABLE);
	lua_getfield(L, index, "__self");
	b2Joint* joint = (b2Joint*) lua_touserdata(L, -1);
	return joint;
}
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

static int mlua_set_target_joint(lua_State* L)
{
	b2MouseJoint* joint = luam_tomousejoint(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	joint->SetTarget(b2Vec2(x, y));
	return 0;
}

static int mlua_set_length_joint(lua_State* L)
{
	b2DistanceJoint* joint = luam_todistancejoint(L, 1);
	lua_Number length = luaL_checknumber(L, 2);
	joint->SetLength(length);
	return 0;
}
static int mlua_set_frequency_joint(lua_State* L)
{
	b2DistanceJoint* joint = luam_todistancejoint(L, 1);
	lua_Number freq = luaL_checknumber(L, 2);
	joint->SetFrequency(freq);
	return 0;
}

static int mlua_set_max_length_joint(lua_State* L)
{
	b2RopeJoint* joint = luam_toropejoint(L, 1);
	lua_Number maxlength = luaL_checknumber(L, 2);
	joint->SetMaxLength(maxlength);
	return 0;
}

static int mlua_set_angle_limits_joint(lua_State* L)
{
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

static int mlua_set_motor_speed_joint(lua_State* L)
{
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

static int mlua_destroy_joint(lua_State* L)
{
	b2Joint* joint = luam_tojoint(L, 1);
	world->DestroyJoint(joint);
	return 0;
}

BEGIN_MODULE(physic)
	DECLARE_FUNCTION(create_world)

	DECLARE_FUNCTION(new_shape)
	DECLARE_FUNCTION(new_body)
	DECLARE_FUNCTION(new_joint)

	DECLARE_FUNCTION(update_physic)
	DECLARE_FUNCTION(on_collision)

	DECLARE_FUNCTION(raycast)
	DECLARE_FUNCTION(query)

	BEGIN_CLASS(body)
		ADD_GETSET(body, position)
		ADD_GETSET(body, angle)
		ADD_GETSET(body, linear_velocity)
		ADD_GETSET(body, angular_velocity)
		ADD_GETSET(body, linear_damping)
		ADD_GETSET(body, angular_damping)
		ADD_GETSET(body, fixed_rotation)

		ADD_METHOD(body, set_active)
		ADD_METHOD(body, set_bullet)
		ADD_METHOD(body, get_mass)
		ADD_METHOD(body, set_mass_center)
		ADD_METHOD(body, apply_force)
		ADD_METHOD(body, apply_linear_impulse)
		ADD_METHOD(body, apply_angular_impulse)
		ADD_METHOD(body, apply_torque)
		ADD_METHOD(body, dump)
		ADD_METHOD(body, destroy)
		END_CLASS();
	REGISTER_CLASS(body, "Body");
	/*
	 * Set field so gamedevs can do:
	 * local MyBody = setmetatable({
	 *     somevars...,
	 * }, physic.Body)
	 * MyBody.__index = MyBody
	 *
	 * ...
	 *
	 * local body = setmetatable(physic.new_body(.., shape), MyBody)
	 * body.somevars = ...
	 */

	BEGIN_CLASS(shape)
		ADD_GETSET(shape, density)
		ADD_GETSET(shape, restitution)
		ADD_GETSET(shape, friction)
		ADD_METHOD(shape, set_sensor)
		ADD_GC(gc_shape)
		END_CLASS();
	REGISTER_CLASS(shape, "Shape");

	BEGIN_CLASS(joint)
		ADD_METHOD(joint, destroy)
		// mouse joint
		ADD_METHOD(joint, set_target)
		// distance joint
		ADD_METHOD(joint, set_length)
		ADD_METHOD(joint, set_frequency)
		// rope joint
		ADD_METHOD(joint, set_max_length)
		// revolute joint
		ADD_METHOD(joint, set_angle_limits)
		ADD_METHOD(joint, set_motor_speed)
		END_CLASS();
	REGISTER_CLASS(joint, "Joint");
END_MODULE()

