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
#include <cstring>

#include "module.hpp"
#include "engine.hpp"
#include "display_bind.hpp"
#include "buffer_bind.hpp"
#include "lua_functions.hpp"
#include "api.hpp"

DECLARE_PUSH(Surface, surface)

BEGIN_MODULE(graphics)
	DECLARE_FUNCTION(show_cursor)

	DECLARE_FUNCTION(resize)
	DECLARE_FUNCTION(set_title)
	DECLARE_FUNCTION(set_fullscreen)
	DECLARE_FUNCTION(screen2scene)

	/* DISPLAY SURFACE */
	DECLARE_FUNCTION(load_surface)
	DECLARE_FUNCTION(new_surface)

	/* DISPLAY DRAWERS */
	DECLARE_FUNCTION(draw_background)
	DECLARE_FUNCTION(draw_point)
	DECLARE_FUNCTION(draw_point_tex)
	DECLARE_FUNCTION(draw_line)
	DECLARE_FUNCTION(draw_triangle)
	DECLARE_FUNCTION(draw_surface)
	DECLARE_FUNCTION(draw_quad)

	/* DISPLAY SETTERS */
	DECLARE_FUNCTION(set_color)
	DECLARE_FUNCTION(set_alpha)
	DECLARE_FUNCTION(set_point_size)
	DECLARE_FUNCTION(set_line_width)
	DECLARE_FUNCTION(set_blend_mode)

	/* DISPLAY SHADER */
	DECLARE_FUNCTION(new_shader)
	DECLARE_FUNCTION(use_shader)

	/* DISPLAY BUFFER */
	DECLARE_FUNCTION(new_buffer)
	DECLARE_FUNCTION(use_buffer)

	BEGIN_CLASS(surface)
		ADD_METHOD(surface, set_filter)
		ADD_METHOD(surface, draw_on)
		ADD_METHOD(surface, draw_from)
		ADD_GC(free_surface)
	REGISTER_CLASS_WITH_INDEX(surface, "Surface")

	BEGIN_CLASS(buffer)
	    ADD_METHOD(buffer, use)
	    ADD_METHOD(buffer, draw)
	    ADD_METHOD(buffer, reset)
	    ADD_METHOD(buffer, upload_and_free)
	    ADD_GC(free_buffer)
	REGISTER_CLASS(buffer, "Buffer")

	BEGIN_CLASS(shader)
	    ADD_METHOD(shader, use)
	    ADD_METHOD(shader, feed)
	    ADD_GC(free_shader)
	REGISTER_CLASS(shader, "Shader")

	{
		// make sure we don't free the screen until the next resize
		push_surface(L, get_engine().display.get_screen());
		lua_setfield(L, LUA_REGISTRYINDEX, "screen");
	}
	{
		// blend modes
		lua_pushnumber(L, DEFAULT);
		lua_setfield(L, -2, "BLEND_DEFAULT");
		lua_pushnumber(L, ALPHA);
		lua_setfield(L, -2, "BLEND_ALPHA");
		lua_pushnumber(L, ADD);
		lua_setfield(L, -2, "BLEND_ADD");
		lua_pushnumber(L, MULT);
		lua_setfield(L, -2, "BLEND_MULT");
	}
	{
		// filter modes
		lua_pushnumber(L, NEAREST);
		lua_setfield(L, -2, "NEAREST");
		lua_pushnumber(L, LINEAR);
		lua_setfield(L, -2, "LINEAR");
		lua_pushnumber(L, BILINEAR);
		lua_setfield(L, -2, "BILINEAR");
		lua_pushnumber(L, TRILINEAR);
		lua_setfield(L, -2, "TRILINEAR");
	}

	{
		// camera
		lua_newtable(L);
		luaL_newmetatable(L, "__camera_class");
		lua_pushcfunction(L, mlua_camera__newindex);
		lua_setfield(L, -2, "__newindex");
		lua_pushcfunction(L, mlua_camera__index);
		lua_setfield(L, -2, "__index");
		lua_setmetatable(L, -2);
		lua_pushcfunction(L, mlua_camera_reset);
		lua_setfield(L, -2, "reset");

		// glue it on drystal table
		lua_setfield(L, -2, "camera");
	}
END_MODULE()

int graphics_index(lua_State* L)
{
	Engine &engine = get_engine();
	const char * name = luaL_checkstring(L, 2);
	if (!strcmp(name, "screen")) {
		Surface* surf = engine.display.get_screen();
		if (surf) {
			push_surface(L, surf);
			return 1;
		}
	} else if (!strcmp(name, "current_draw_on")) {
		Surface* surf = engine.display.get_draw_on();
		if (surf) {
			push_surface(L, surf);
			return 1;
		}
	} else if (!strcmp(name, "current_draw_from")) {
		Surface* surf = engine.display.get_draw_from();
		if (surf) {
			push_surface(L, surf);
			return 1;
		}
	}

	return 0;
}

