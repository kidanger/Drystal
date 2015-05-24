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
#include <assert.h>
#include <errno.h>
#include <lua.h>
#include <lauxlib.h>

#include "display.h"
#include "buffer.h"
#include "display_bind.h"
#include "lua_util.h"
#include "dlua.h"
#include "log.h"
#include "util.h"

log_category("graphics");

IMPLEMENT_PUSHPOP(Surface, surface)

int mlua_set_color(lua_State* L)
{
	assert(L);

	if (lua_isstring(L, 1) && !lua_isnumber(L, 1)) {
		dlua_get_drystal_field("colors");
		lua_pushvalue(L, 1);
		lua_gettable(L, -2);
		lua_replace(L, 1);
	}

	if (lua_istable(L, 1)) {
		lua_rawgeti(L, 1, 1);
		lua_rawgeti(L, 1, 2);
		lua_rawgeti(L, 1, 3);
	}
	lua_Integer r = luaL_checknumber(L, -3);
	lua_Integer g = luaL_checknumber(L, -2);
	lua_Integer b = luaL_checknumber(L, -1);

	assert_lua_error(L, r >= 0 && r <= 255, "set_color: the red component must be >= 0 and <= 255");
	assert_lua_error(L, g >= 0 && g <= 255, "set_color: the green component must be >= 0 and <= 255");
	assert_lua_error(L, b >= 0 && b <= 255, "set_color: the blue component must be >= 0 and <= 255");

	display_set_color(r, g, b);
	return 0;
}

int mlua_set_alpha(lua_State* L)
{
	assert(L);

	lua_Integer alpha = luaL_checknumber(L, 1);

	assert_lua_error(L, alpha >= 0 && alpha <= 255, "set_alpha: the alpha component must be >= 0 and <= 255");

	display_set_alpha(alpha);
	return 0;
}

int mlua_set_line_width(lua_State* L)
{
	assert(L);

	lua_Number width = luaL_checknumber(L, 1);

	assert_lua_error(L, width >= 0, "set_line_width: must be >= 0");

	display_set_line_width(width);
	return 0;
}

int mlua_set_fullscreen(lua_State* L)
{
	assert(L);

	bool fullscreen = lua_toboolean(L, 1);
	display_set_fullscreen(fullscreen);

	push_surface(L, display_get_screen());
	lua_setfield(L, LUA_REGISTRYINDEX, "screen");
	return 0;
}

int mlua_set_title(lua_State* L)
{
	assert(L);

	const char *title = luaL_checkstring(L, 1);
	display_set_title(title);
	return 0;
}

int mlua_set_blend_mode(lua_State* L)
{
	assert(L);

	BlendMode mode = (BlendMode) luaL_checkinteger(L, 1);
	display_set_blend_mode(mode);
	return 0;
}

int mlua_show_cursor(lua_State* L)
{
	assert(L);

	bool show = lua_toboolean(L, 1);
	display_show_cursor(show);
	return 0;
}

int mlua_resize(lua_State* L)
{
	assert(L);

	int w = luaL_checkinteger(L, 1);
	int h = luaL_checkinteger(L, 2);

	assert_lua_error(L, w > 0, "resize: width must be > 0");
	assert_lua_error(L, h > 0, "resize: height must be > 0");

	display_resize(w, h);

	// make sure we don't free the screen until the next resize
	push_surface(L, display_get_screen());
	lua_setfield(L, LUA_REGISTRYINDEX, "screen");
	return 0;
}

int mlua_screen2scene(lua_State* L)
{
	assert(L);

	lua_Number x = luaL_checknumber(L, 1);
	lua_Number y = luaL_checknumber(L, 2);
	float tx, ty;
	display_screen2scene(x, y, &tx, &ty);
	lua_pushnumber(L, tx);
	lua_pushnumber(L, ty);
	return 2;
}

int mlua_surface_class_index(lua_State* L)
{
	assert(L);

	Surface* surface = pop_surface(L, 1);
	const char* index = luaL_checkstring(L, 2);
	if (streq(index, "w")) {
		lua_pushnumber(L, surface->w);
	} else if (streq(index, "h")) {
		lua_pushnumber(L, surface->h);
	} else {
		lua_getmetatable(L, 1);
		lua_getfield(L, -1, index);
	}
	return 1;
}

int mlua_load_surface(lua_State* L)
{
	assert(L);

	int r;
	Surface *surface;
	const char * filename = luaL_checkstring(L, 1);
	r = display_load_surface(filename, &surface);
	if (r < 0) {
		lua_pushnil(L);
		if (r == -E2BIG) {
			lua_pushliteral(L, "load_surface: surface size must be width > 0 and <= 2048, height > 0 and <= 2048");
		} else if (r == -ENOTSUP) {
			lua_pushliteral(L, "load_surface: unsupported format");
		} else if (r == -EBADMSG) {
			lua_pushliteral(L, "load_surface: not a PNG");
		} else {
			lua_pushfstring(L, "%s: %s", "load_surface", strerror(errno));
		}
		return 2;
	}
	push_surface(L, surface);
	return 1;
}

int mlua_new_surface(lua_State* L)
{
	assert(L);

	int w = luaL_checkinteger(L, 1);
	int h = luaL_checkinteger(L, 2);

	assert_lua_error(L, w > 0 && w <= 2048, "new_surface: width must be > 0 and <= 2048");
	assert_lua_error(L, h > 0 && h <= 2048, "new_surface: height must be > 0 and <= 2048");

	bool force_npot = lua_toboolean(L, 3);
	Surface* surface = display_new_surface(w, h, force_npot);
	push_surface(L, surface);
	return 1;
}

int mlua_free_surface(lua_State* L)
{
	assert(L);

	Surface* surface = pop_surface(L, 1);
	display_free_surface(surface);
	return 0;
}

int mlua_draw_on_surface(lua_State* L)
{
	assert(L);

	Surface* old = display_get_draw_on();
	Surface* surface = pop_surface(L, 1);
	display_draw_on(surface);

	if (old) {
		push_surface(L, old);
		return 1;
	}
	return 0;
}

int mlua_draw_from_surface(lua_State* L)
{
	assert(L);

	Surface* old = display_get_draw_from();
	Surface* surface = pop_surface(L, 1);
	display_draw_from(surface);

	if (old) {
		push_surface(L, old);
		return 1;
	}
	return 0;
}

int mlua_set_filter_surface(lua_State* L)
{
	assert(L);

	Surface* surface = pop_surface(L, 1);
	FilterMode mode = (FilterMode) luaL_checkinteger(L, 2);
	assert_lua_error(L, mode == FILTER_NEAREST || mode == FILTER_LINEAR
					 || mode == FILTER_BILINEAR || mode == FILTER_TRILINEAR,
					 "set_filter: invalid filter (drystal.filters.nearest, linear/default, bilinear, trilinear)");
	assert_lua_error(L, !surface->npot || mode == FILTER_NEAREST || mode == FILTER_LINEAR,
					 "set_filter: invalid filter for NPOT texture");
	display_set_filter(surface, mode);
	return 0;
}

int mlua_get_pixel_surface(lua_State* L)
{
	assert(L);

	Surface* surface = pop_surface(L, 1);
	unsigned int x = luaL_checkinteger(L, 2);
	unsigned int y = luaL_checkinteger(L, 3);
	assert_lua_error(L, surface != display_get_draw_on(), "get_pixel: the surface is currently drawn on");
	assert_lua_error(L, x >= 1, "get_pixel: x must be greater or equal to 1");
	assert_lua_error(L, y >= 1, "get_pixel: y must be greater or equal to 1");
	assert_lua_error(L, x <= surface->w, "get_pixel: x must be equal or less than texture's width");
	assert_lua_error(L, y <= surface->h, "get_pixel: y must be equal or less than texture's height");

	lua_Integer r, g, b;
	lua_Integer a;
	display_get_pixel(surface, x - 1, y - 1, &r, &g, &b, &a);

	lua_pushinteger(L, r);
	lua_pushinteger(L, g);
	lua_pushinteger(L, b);
	lua_pushinteger(L, a);
	return 4;
}

int mlua_draw_background(_unused_ lua_State *L)
{
	display_draw_background();
	return 0;
}

int mlua_draw_point(lua_State* L)
{
	assert(L);

	Buffer* buffer = display_get_current_buffer();
	if (buffer->user_buffer) {
		if (display_is_debug()) {
		} else if (buffer->size > 0 && buffer->has_texture)
			return luaL_error(L, "the current buffer cannot contain non-textured points");
	}

	lua_Number x = luaL_checknumber(L, 1);
	lua_Number y = luaL_checknumber(L, 2);
	lua_Number size = luaL_checknumber(L, 3);
	display_draw_point(x, y, size);
	return 0;
}

int mlua_draw_point_tex(lua_State* L)
{
	assert(L);

	assert_lua_error(L, display_get_draw_from(), "draw_point_tex: no 'drawn from' surface bound");

	Buffer* buffer = display_get_current_buffer();
	if (buffer->user_buffer) {
		if (display_is_debug()) {
		} else if (buffer->size > 0 && !buffer->has_texture)
			return luaL_error(L, "the current buffer cannot contain textured points");
	}

	lua_Number sx = luaL_checknumber(L, 1);
	lua_Number sy = luaL_checknumber(L, 2);
	lua_Number x = luaL_checknumber(L, 3);
	lua_Number y = luaL_checknumber(L, 4);
	lua_Number size = luaL_checknumber(L, 5);
	display_draw_point_tex(sx, sy, x, y, size);
	return 0;
}

int mlua_draw_line(lua_State* L)
{
	assert(L);

	Buffer* buffer = display_get_current_buffer();
	if (buffer->user_buffer) {
		if (buffer->size > 0 &&  buffer->has_texture)
			return luaL_error(L, "the current buffer cannot contain lines");
	}

	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);
	display_draw_line(x1, y1, x2, y2);
	return 0;
}

int mlua_draw_triangle(lua_State* L)
{
	assert(L);

	Buffer* buffer = display_get_current_buffer();
	if (buffer->user_buffer) {
		if (display_is_debug()) {
		} else if (buffer->size > 0 && buffer->has_texture)
			return luaL_error(L, "the current buffer cannot contain non-textured triangles");
	}

	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);
	lua_Number x3 = luaL_checknumber(L, 5);
	lua_Number y3 = luaL_checknumber(L, 6);
	display_draw_triangle(x1, y1, x2, y2, x3, y3);
	return 0;
}

int mlua_draw_surface(lua_State* L)
{
	assert(L);

	assert_lua_error(L, display_get_draw_from(), "draw_surface: no 'drawn from' surface bound");

	Buffer* buffer = display_get_current_buffer();
	if (buffer->user_buffer) {
		if (display_is_debug()) {
		} else if (buffer->size > 0 && !buffer->has_texture)
			return luaL_error(L, "the current buffer cannot contain textured triangles");
	}

	lua_Number i1 = luaL_checknumber(L, 1);
	lua_Number i2 = luaL_checknumber(L, 2);
	lua_Number i3 = luaL_checknumber(L, 3);
	lua_Number i4 = luaL_checknumber(L, 4);
	lua_Number i5 = luaL_checknumber(L, 5);
	lua_Number i6 = luaL_checknumber(L, 6);
	lua_Number o1 = luaL_checknumber(L, 7);
	lua_Number o2 = luaL_checknumber(L, 8);
	lua_Number o3 = luaL_checknumber(L, 9);
	lua_Number o4 = luaL_checknumber(L, 10);
	lua_Number o5 = luaL_checknumber(L, 11);
	lua_Number o6 = luaL_checknumber(L, 12);
	display_draw_surface(i1, i2, i3, i4, i5, i6,
	                     o1, o2, o3, o4, o5, o6);
	return 0;
}

int mlua_draw_quad(lua_State* L)
{
	assert(L);

	assert_lua_error(L, display_get_draw_from(), "draw_quad: no 'drawn from' surface bound");

	Buffer* buffer = display_get_current_buffer();
	if (buffer->user_buffer) {
		if (display_is_debug()) {
		} else if (buffer->size > 0 && !buffer->has_texture)
			return luaL_error(L, "the current buffer cannot contain textured triangles");
	}

	lua_Number i1 = luaL_checknumber(L, 1);
	lua_Number i2 = luaL_checknumber(L, 2);
	lua_Number i3 = luaL_checknumber(L, 3);
	lua_Number i4 = luaL_checknumber(L, 4);
	lua_Number i5 = luaL_checknumber(L, 5);
	lua_Number i6 = luaL_checknumber(L, 6);
	lua_Number i7 = luaL_checknumber(L, 7);
	lua_Number i8 = luaL_checknumber(L, 8);
	lua_Number o1 = luaL_checknumber(L, 9);
	lua_Number o2 = luaL_checknumber(L, 10);
	lua_Number o3 = luaL_checknumber(L, 11);
	lua_Number o4 = luaL_checknumber(L, 12);
	lua_Number o5 = luaL_checknumber(L, 13);
	lua_Number o6 = luaL_checknumber(L, 14);
	lua_Number o7 = luaL_checknumber(L, 15);
	lua_Number o8 = luaL_checknumber(L, 16);
	display_draw_quad(i1, i2, i3, i4, i5, i6, i7, i8,
	                  o1, o2, o3, o4, o5, o6, o7, o8);
	return 0;
}

