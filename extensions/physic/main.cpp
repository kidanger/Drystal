#include <lua.hpp>

#include "engine.hpp"

#include "box2d/Box2D/Box2D.h"

#define BODY_CLASS "__body_class"
#define SHAPE_CLASS "__shape_class"
#define JOINT_CLASS "__joint_class"

#define DECLARE_FUNCTION(x) {#x, x}
#define DECLARE_GETSET(x) DECLARE_FUNCTION(set_##x), DECLARE_FUNCTION(get_##x)

b2World* world;

int create_world(lua_State* L)
{
	lua_Number gravity_x = luaL_checknumber(L, 1);
	lua_Number gravity_y = luaL_checknumber(L, 2);
	world = new b2World(b2Vec2(gravity_x, gravity_y));
	return 0;
}

int update(lua_State* L)
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
	virtual void BeginContact(b2Contact* contact)
	{
		if (begin_contact == LUA_REFNIL)
			return;
		b2Body* bA = contact->GetFixtureA()->GetBody();
		b2Body* bB = contact->GetFixtureB()->GetBody();

		lua_rawgeti(L, LUA_REGISTRYINDEX, begin_contact);

		// fetch bodies tables
		int refA = (int) (size_t) bA->GetUserData();
		lua_rawgeti(L, LUA_REGISTRYINDEX, refA);
		int refB = (int) (size_t) bB->GetUserData();
		lua_rawgeti(L, LUA_REGISTRYINDEX, refB);
		// TODO: push contact

		if (lua_pcall(L, 2, 0, 0)) {
			luaL_error(L, "error calling begin_contact: %s", lua_tostring(L, -1));
		}
	}
	virtual void EndContact(b2Contact* contact)
	{
		if (end_contact == LUA_REFNIL)
			return;
		b2Body* bA = contact->GetFixtureA()->GetBody();
		b2Body* bB = contact->GetFixtureB()->GetBody();

		lua_rawgeti(L, LUA_REGISTRYINDEX, end_contact);

		// fetch bodies tables
		int refA = (int) (long) bA->GetUserData();
		lua_rawgeti(L, LUA_REGISTRYINDEX, refA);
		int refB = (int) (long) bB->GetUserData();
		lua_rawgeti(L, LUA_REGISTRYINDEX, refB);
		// TODO: push contact

		if (lua_pcall(L, 2, 0, 0)) {
			luaL_error(L, "error calling end_contact: %s", lua_tostring(L, -1));
		}
	}

	virtual void PreSolve(b2Contact* contact, const b2Manifold*)
	{
		if (presolve == LUA_REFNIL)
			return;
		b2Body* bA = contact->GetFixtureA()->GetBody();
		b2Body* bB = contact->GetFixtureB()->GetBody();

		lua_rawgeti(L, LUA_REGISTRYINDEX, presolve);

		// fetch bodies tables
		int refA = (int) (long) bA->GetUserData();
		lua_rawgeti(L, LUA_REGISTRYINDEX, refA);
		int refB = (int) (long) bB->GetUserData();
		lua_rawgeti(L, LUA_REGISTRYINDEX, refB);
		// TODO: push contact

		if (lua_pcall(L, 2, 1, 0)) {
			luaL_error(L, "error calling presolve: %s", lua_tostring(L, -1));
		}
		bool enabled = lua_toboolean(L, -1);
		contact->SetEnabled(enabled);
	}
	virtual void PostSolve(b2Contact* contact, const b2ContactImpulse*)
	{
		if (postsolve == LUA_REFNIL)
			return;
		b2Body* bA = contact->GetFixtureA()->GetBody();
		b2Body* bB = contact->GetFixtureB()->GetBody();

		lua_rawgeti(L, LUA_REGISTRYINDEX, postsolve);

		// fetch bodies tables
		int refA = (int) (long) bA->GetUserData();
		lua_rawgeti(L, LUA_REGISTRYINDEX, refA);
		int refB = (int) (long) bB->GetUserData();
		lua_rawgeti(L, LUA_REGISTRYINDEX, refB);
		// TODO: push contact

		if (lua_pcall(L, 2, 1, 0)) {
			luaL_error(L, "error calling postsolve: %s", lua_tostring(L, -1));
		}
	}
};
int on_collision(lua_State* L)
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

int raycast(lua_State* L)
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
	if (x1 != x2 and y1 != y2) {
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

int query(lua_State* L)
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

int new_shape(lua_State* L)
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
	luaL_getmetatable(L, SHAPE_CLASS);
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
	int shape_set_##value(lua_State* L) \
	{ \
		b2FixtureDef* fixtureDef = luam_tofixture(L, 1); \
		lua_Number value = luaL_checknumber(L, 2); \
		fixtureDef->value = value; \
		return 0; \
	} \
	int shape_get_##value(lua_State* L) \
	{ \
		b2FixtureDef* fixtureDef = luam_tofixture(L, 1); \
		lua_pushnumber(L, fixtureDef->value); \
		return 1; \
	}
SHAPE_GETSET_SOME_VALUE(density)
SHAPE_GETSET_SOME_VALUE(restitution)
SHAPE_GETSET_SOME_VALUE(friction)

int shape_set_sensor(lua_State* L)
{
	b2FixtureDef* fixtureDef = luam_tofixture(L, 1);
	bool sensor = lua_toboolean(L, 2);
	fixtureDef->isSensor = sensor;
	return 0;
}

int shape_gc(lua_State* L)
{
	b2FixtureDef* fixtureDef = luam_tofixture(L, 1);
	delete fixtureDef->shape;
	delete fixtureDef;
	return 0;
}

#define DECLARE_SHAPE_FUNCTION(name) {#name, shape_##name}
#define DECLARE_SHAPE_GETSET(x) DECLARE_SHAPE_FUNCTION(set_##x), DECLARE_SHAPE_FUNCTION(get_##x)

static const luaL_Reg __shape_class[] = {
	DECLARE_SHAPE_GETSET(density),
	DECLARE_SHAPE_GETSET(restitution),
	DECLARE_SHAPE_GETSET(friction),
	DECLARE_SHAPE_FUNCTION(set_sensor),

	{"__gc", shape_gc},
	{NULL, NULL},
};


// Body methods

int new_body(lua_State* L)
{
	assert(world);

	bool dynamic = lua_toboolean(L, 1);

	int number_of_shapes = lua_gettop(L) - 1;
	b2FixtureDef* fixtureDefs[number_of_shapes];
	for (int i = 0; i < number_of_shapes; i++) {
		fixtureDefs[i] = luam_tofixture(L, i + 1 + 1);
		assert(fixtureDefs[i]);
		assert(fixtureDefs[i]->shape);
	}

	b2BodyDef def;
	if (dynamic) {
		def.type = b2_dynamicBody;
	}

	b2Body* body = world->CreateBody(&def);
	for (int i = 0; i < number_of_shapes; i++) {
		body->CreateFixture(fixtureDefs[i]);
	}

	lua_newtable(L);
	lua_pushlightuserdata(L, body);
	lua_setfield(L, -2, "__self");
	luaL_getmetatable(L, BODY_CLASS);
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
	int set_##value(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
		lua_Number x = luaL_checknumber(L, 2); \
		lua_Number y = luaL_checknumber(L, 3); \
		b2Vec2 vector(x, y); \
		set_expr; \
		return 0; \
	} \
	int get_##value(lua_State* L) \
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
	int set_##value(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
		lua_Number value = luaL_checknumber(L, 2); \
		set_expr; \
		return 0; \
	} \
	int get_##value(lua_State* L) \
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

int get_mass(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	const lua_Number mass = body->GetMass();
	lua_pushnumber(L, mass);
	return 1;
}

int set_mass_center(lua_State* L)
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
	int set_##value(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
		bool value = lua_toboolean(L, 2); \
		set_expr; \
		return 0; \
	} \
	int get_##value(lua_State* L) \
	{ \
		b2Body* body = luam_tobody(L, 1); \
		const bool value = get_expr; \
		lua_pushboolean(L, value); \
		return 1; \
	}

BODY_GETSET_BOOL(fixed_rotation, body->IsFixedRotation(), body->SetFixedRotation(fixed_rotation))

static int apply_force(lua_State* L)
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
static int apply_linear_impulse(lua_State* L)
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
static int apply_angular_impulse(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	lua_Number angle = luaL_checknumber(L, 2);
	body->ApplyAngularImpulse(angle, true);
	return 0;
}
static int apply_torque(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	lua_Number torque = luaL_checknumber(L, 2);
	body->ApplyTorque(torque, true);
	return 0;
}

static int dump(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	body->Dump();
	return 0;
}

static int body_destroy(lua_State* L)
{
	b2Body* body = luam_tobody(L, 1);
	world->DestroyBody(body);
	return 0;
}


static const luaL_Reg __body_class[] = {
	DECLARE_GETSET(position),
	DECLARE_GETSET(angle),
	DECLARE_GETSET(linear_velocity),
	DECLARE_GETSET(angular_velocity),
	DECLARE_GETSET(linear_damping),
	DECLARE_GETSET(angular_damping),
	DECLARE_GETSET(fixed_rotation),
	DECLARE_FUNCTION(get_mass),
	DECLARE_FUNCTION(set_mass_center),
	DECLARE_FUNCTION(apply_force),
	DECLARE_FUNCTION(apply_linear_impulse),
	DECLARE_FUNCTION(apply_angular_impulse),
	DECLARE_FUNCTION(apply_torque),
	DECLARE_FUNCTION(dump),
	{"destroy", body_destroy},
	{NULL, NULL},
};

// Joint methods

int new_joint(lua_State* L)
{
	assert(world);

	b2JointDef* joint_def;

	const char * type = luaL_checkstring(L, 1);
	int i = 2;
	if (!strcmp(type, "mouse")) {
		b2MouseJointDef* def = new b2MouseJointDef;
		def->bodyA = luam_tobody(L, i++);
		def->bodyB = luam_tobody(L, i++);
		def->maxForce = lua_tonumber(L, i++);
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
	luaL_getmetatable(L, JOINT_CLASS);
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

int set_target(lua_State* L)
{
	b2MouseJoint* joint = luam_tomousejoint(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	joint->SetTarget(b2Vec2(x, y));
	return 0;
}

int set_length(lua_State* L)
{
	b2DistanceJoint* joint = luam_todistancejoint(L, 1);
	lua_Number length = luaL_checknumber(L, 2);
	joint->SetLength(length);
	return 0;
}
int set_frequency(lua_State* L)
{
	b2DistanceJoint* joint = luam_todistancejoint(L, 1);
	lua_Number freq = luaL_checknumber(L, 2);
	joint->SetFrequency(freq);
	return 0;
}

int set_max_length(lua_State* L)
{
	b2RopeJoint* joint = luam_toropejoint(L, 1);
	lua_Number maxlength = luaL_checknumber(L, 2);
	joint->SetMaxLength(maxlength);
	return 0;
}

int set_angle_limits(lua_State* L)
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

int set_motor_speed(lua_State* L)
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

int destroy(lua_State* L)
{
	b2Joint* joint = luam_tojoint(L, 1);
	world->DestroyJoint(joint);
	return 0;
}

static const luaL_Reg __joint_class[] = {
	// joint
	DECLARE_FUNCTION(destroy),
	// mouse joint
	DECLARE_FUNCTION(set_target),
	// distance joint
	DECLARE_FUNCTION(set_length),
	DECLARE_FUNCTION(set_frequency),
	// rope joint
	DECLARE_FUNCTION(set_max_length),
	// revolute joint
	DECLARE_FUNCTION(set_angle_limits),
	DECLARE_FUNCTION(set_motor_speed),
	{NULL, NULL},
};

// Physic module

static const luaL_Reg lib[] =
{
	DECLARE_FUNCTION(create_world),

	DECLARE_FUNCTION(new_shape),
	DECLARE_FUNCTION(new_body),
	DECLARE_FUNCTION(new_joint),

	DECLARE_FUNCTION(update),
	DECLARE_FUNCTION(on_collision),

	DECLARE_FUNCTION(raycast),
	DECLARE_FUNCTION(query),

	{NULL, NULL}
};

DEFINE_EXTENSION(physic)
{
	luaL_newlib(L, lib);
	luaL_setfuncs(L, lib, 0);

	// register BODY_CLASS
	luaL_newmetatable(L, BODY_CLASS);
	luaL_setfuncs(L, __body_class, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_setfield(L, -2, BODY_CLASS);

	// register SHAPE_CLASS
	luaL_newmetatable(L, SHAPE_CLASS);
	luaL_setfuncs(L, __shape_class, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_setfield(L, -2, SHAPE_CLASS);

	// register JOINT_CLASS
	luaL_newmetatable(L, JOINT_CLASS);
	luaL_setfuncs(L, __joint_class, 0);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");
	lua_setfield(L, -2, JOINT_CLASS);

	return 1;
}

