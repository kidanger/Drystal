#include <cassert>
#include <lua.hpp>

#include "macro.hpp"

DISABLE_WARNING_EFFCPP;
#include <Box2D/Box2D.h>
REENABLE_WARNING;

#include "physic_p.hpp"

b2Body* luam_tobody(lua_State* L, int index)
{
	assert(L);

	luaL_checktype(L, index, LUA_TTABLE);
	lua_getfield(L, index, "__self");
	b2Body* body = (b2Body*) lua_touserdata(L, -1);
	return body;
}

b2FixtureDef* luam_tofixture(lua_State* L, int index)
{
	assert(L);

	luaL_checktype(L, index, LUA_TTABLE);
	lua_getfield(L, index, "__self");
	b2FixtureDef* shape = (b2FixtureDef*) lua_touserdata(L, -1);
	return shape;
}

b2Joint* luam_tojoint(lua_State* L, int index)
{
	assert(L);

	luaL_checktype(L, index, LUA_TTABLE);
	lua_getfield(L, index, "__self");
	b2Joint* joint = (b2Joint*) lua_touserdata(L, -1);
	return joint;
}

