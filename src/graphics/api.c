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
#include <lua.h>
#include <lauxlib.h>

#include "module.h"
#include "display_bind.h"
#include "display.h"
#include "buffer.h"
#include "camera_bind.h"
#include "shader_bind.h"
#include "buffer_bind.h"
#include "api.h"
#include "util.h"

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
	DECLARE_FUNCTION(set_line_width)
	DECLARE_FUNCTION(set_blend_mode)

	BEGIN_CLASS(surface)
		ADD_METHOD(surface, set_filter)
		ADD_METHOD(surface, draw_on)
		ADD_METHOD(surface, draw_from)
		ADD_GC(free_surface)
	REGISTER_CLASS_WITH_INDEX(surface, "Surface")

	DECLARE_FUNCTION(new_buffer)
	DECLARE_FUNCTION(use_default_buffer)
	BEGIN_CLASS(buffer)
	    ADD_METHOD(buffer, use)
	    ADD_METHOD(buffer, draw)
	    ADD_METHOD(buffer, reset)
	    ADD_METHOD(buffer, upload_and_free)
	    ADD_GC(free_buffer)
	REGISTER_CLASS(buffer, "Buffer")

	DECLARE_FUNCTION(new_shader)
	DECLARE_FUNCTION(use_default_shader)
	BEGIN_CLASS(shader)
	    ADD_METHOD(shader, use)
	    ADD_METHOD(shader, feed)
	    ADD_GC(free_shader)
	REGISTER_CLASS(shader, "Shader")

	{
		// make sure we don't free the screen until the next resize
		push_surface(L, display_get_screen());
		lua_setfield(L, LUA_REGISTRYINDEX, "screen");
	}
	// blend modes
	DECLARE_CONSTANT_WITH_NAME(BLEND_DEFAULT,    "BLEND_DEFAULT")
	DECLARE_CONSTANT_WITH_NAME(BLEND_ALPHA,      "BLEND_ALPHA")
	DECLARE_CONSTANT_WITH_NAME(BLEND_ADD,        "BLEND_ADD")
	DECLARE_CONSTANT_WITH_NAME(BLEND_MULT,       "BLEND_MULT")
	// filter modes
	DECLARE_CONSTANT_WITH_NAME(FILTER_NEAREST,   "NEAREST")
	DECLARE_CONSTANT_WITH_NAME(FILTER_LINEAR,    "LINEAR")
	DECLARE_CONSTANT_WITH_NAME(FILTER_BILINEAR,  "BILINEAR")
	DECLARE_CONSTANT_WITH_NAME(FILTER_TRILINEAR, "TRILINEAR")
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
	const char * name = luaL_checkstring(L, 2);
	if (streq(name, "screen")) {
		Surface* surf = display_get_screen();
		if (surf) {
			push_surface(L, surf);
			return 1;
		}
	} else if (streq(name, "current_draw_on")) {
		Surface* surf = display_get_draw_on();
		if (surf) {
			push_surface(L, surf);
			return 1;
		}
	} else if (streq(name, "current_draw_from")) {
		Surface* surf = display_get_draw_from();
		if (surf) {
			push_surface(L, surf);
			return 1;
		}
	}

	return 0;
}

