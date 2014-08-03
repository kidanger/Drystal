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
#include "camera_bind.hpp"

int mlua_camera__newindex(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	const char * name = luaL_checkstring(L, 2);
	if (!strcmp(name, "x")) {
		lua_Number dx = luaL_checknumber(L, 3);
		engine.display.set_camera_position(dx, engine.display.get_camera().dy);
	} else if (!strcmp(name, "y")) {
		lua_Number dy = luaL_checknumber(L, 3);
		engine.display.set_camera_position(engine.display.get_camera().dx, dy);
	} else if (!strcmp(name, "angle")) {
		lua_Number angle = luaL_checknumber(L, 3);
		engine.display.set_camera_angle(angle);
	} else if (!strcmp(name, "zoom")) {
		lua_Number zoom = luaL_checknumber(L, 3);
		engine.display.set_camera_zoom(zoom);
	} else {
		lua_rawset(L, 1);
	}
	return 0;
}

int mlua_camera__index(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	const char * name = luaL_checkstring(L, 2);
	if (!strcmp(name, "x")) {
		lua_Number dx = engine.display.get_camera().dx;
		lua_pushnumber(L, dx);
		return 1;
	} else if (!strcmp(name, "y")) {
		lua_Number dy = engine.display.get_camera().dy;
		lua_pushnumber(L, dy);
		return 1;
	} else if (!strcmp(name, "angle")) {
		lua_Number angle = engine.display.get_camera().angle;
		lua_pushnumber(L, angle);
		return 1;
	} else if (!strcmp(name, "zoom")) {
		lua_Number zoom = engine.display.get_camera().zoom;
		lua_pushnumber(L, zoom);
		return 1;
	}
	return 0;
}

int mlua_camera_reset(lua_State*)
{
	Engine &engine = get_engine();
	engine.display.reset_camera();
	return 0;
}
