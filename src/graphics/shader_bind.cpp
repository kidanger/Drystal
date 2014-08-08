/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option); any later version.
 *
 * Drystal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Drystal.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cstring>
#include <cassert>
#include <lua.hpp>

#include "engine.hpp"
#include "shader_bind.hpp"
#include "shader.hpp"
#include "log.hpp"

log_category("shader");

IMPLEMENT_PUSHPOP(Shader, shader)

int mlua_new_shader(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	const char *vert = NULL, *frag_color = NULL, *frag_tex = NULL;
	// strings can be nil
	if (lua_gettop(L) >= 1) { // one argument, it's the vertex shader
		vert = lua_tostring(L, 1);
	}
	if (lua_gettop(L) >= 2) {
		frag_color = lua_tostring(L, 2);
	}
	if (lua_gettop(L) >= 3) {
		frag_tex = lua_tostring(L, 3);
	}
	char* error;
	Shader* shader = engine.display.new_shader(vert, frag_color, frag_tex, &error);
	if (shader) {
		push_shader(L, shader);
		return 1;
	} else {
		lua_pushnil(L);
		lua_pushstring(L, error);
		delete[] error;
		return 2;
	}
}

int mlua_use_shader(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Shader* shader = pop_shader(L, -1);
	engine.display.use_shader(shader);
	return 0;
}

int mlua_use_default_shader(_unused_ lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	engine.display.use_default_shader();
	return 0;
}

int mlua_feed_shader(lua_State* L)
{
	assert(L);

	Shader* shader = pop_shader(L, 1);
	const char* name = lua_tostring(L, 2);
	lua_Number value = luaL_checknumber(L, 3);
	shader->feed(name, value);
	return 0;
}

int mlua_free_shader(lua_State* L)
{
	assert(L);

	log_debug("");
	Engine &engine = get_engine();
	Shader* shader = pop_shader(L, 1);
	engine.display.free_shader(shader);
	return 0;
}

