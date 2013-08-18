#define LUA_API extern

#include <lua.hpp>

#include "box2d/Box2D/Box2D.h"

#define BODY_CLASS "__body_class"
#define SHAPE_CLASS "__shape_class"

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

// Shape methods

int new_shape(lua_State* L)
{
	assert(world);

	const char* type = luaL_checkstring(L, 1);

	b2FixtureDef* fixtureDef = new b2FixtureDef;
	if (!strcmp(type, "box")) {
		b2PolygonShape* polygon = new b2PolygonShape;
		lua_Number w = luaL_checknumber(L, 2) / 2;
		lua_Number h = luaL_checknumber(L, 3) / 2;
		polygon->SetAsBox(w, h);
		fixtureDef->shape = polygon;
	} else if (!strcmp(type, "circle")) {
		b2CircleShape* circle = new b2CircleShape;
		lua_Number radius = luaL_checknumber(L, 2);
		circle->m_radius = radius;
		fixtureDef->shape = circle;
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
	int set_##value(lua_State* L) \
	{ \
		b2FixtureDef* fixtureDef = luam_tofixture(L, 1); \
		lua_Number value = luaL_checknumber(L, 2); \
		fixtureDef->value = value; \
		return 0; \
	} \
	int get_##value(lua_State* L) \
	{ \
		b2FixtureDef* fixtureDef = luam_tofixture(L, 1); \
		lua_pushnumber(L, fixtureDef->value); \
		return 1; \
	}
SHAPE_GETSET_SOME_VALUE(density)
SHAPE_GETSET_SOME_VALUE(restitution)
SHAPE_GETSET_SOME_VALUE(friction)

int shape_gc(lua_State* L)
{
	b2FixtureDef* fixtureDef = luam_tofixture(L, 1);
	delete fixtureDef->shape;
	delete fixtureDef;
	printf("[gc] shape\n");
	return 0;
}

static const luaL_Reg __shape_class[] = {
	DECLARE_GETSET(density),
	DECLARE_GETSET(restitution),
	DECLARE_GETSET(friction),

	{"__gc", shape_gc},
	{NULL, NULL},
};


// Body methods

int new_body(lua_State* L)
{
	assert(world);

	b2FixtureDef* fixtureDef = luam_tofixture(L, 1);
	bool dynamic = lua_toboolean(L, 2);

	assert(fixtureDef);
	assert(fixtureDef->shape);

	b2BodyDef def;
	if (dynamic)
		def.type = b2_dynamicBody;

	b2Body* body = world->CreateBody(&def);
	body->CreateFixture(fixtureDef);

	lua_newtable(L);
	lua_pushlightuserdata(L, body);
	lua_setfield(L, -2, "__self");
	luaL_getmetatable(L, BODY_CLASS);
	lua_setmetatable(L, -2);
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
	body->ApplyForce(b2Vec2(fx, fy), body->GetLocalCenter(), true);
	return 0;
}

static const luaL_Reg __body_class[] = {
	DECLARE_GETSET(position),
	DECLARE_GETSET(angle),
	DECLARE_GETSET(linear_velocity),
	DECLARE_GETSET(angular_velocity),
	DECLARE_GETSET(linear_damping),
	DECLARE_GETSET(fixed_rotation),
	{"apply_force", apply_force},
	{NULL, NULL},
};


// Physic module

static const luaL_Reg lib[] =
{
	DECLARE_FUNCTION(create_world),

	DECLARE_FUNCTION(new_shape),
	DECLARE_FUNCTION(new_body),

	DECLARE_FUNCTION(update),

	{NULL, NULL}
};

LUA_API "C" int luaopen_physic(lua_State *L)
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

	lua_setfield(L, -2, BODY_CLASS);

	return 1;
}


