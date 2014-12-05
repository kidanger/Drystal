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
#include <cerrno>
#include <lua.hpp>

#include "engine.hpp"
#include "display_bind.hpp"
#include "lua_util.h"
#include "log.h"

log_category("graphics");

IMPLEMENT_PUSHPOP(Surface, surface)

int mlua_set_color(lua_State* L)
{
	assert(L);

	if (lua_istable(L, 1)) {
		lua_rawgeti(L, 1, 1);
		lua_rawgeti(L, 1, 2);
		lua_rawgeti(L, 1, 3);
	}
	int r = luaL_checkint(L, -3);
	int g = luaL_checkint(L, -2);
	int b = luaL_checkint(L, -1);

	assert_lua_error(L, r >= 0 && r <= 255, "set_color: the red component must be >= 0 and <= 255");
	assert_lua_error(L, g >= 0 && g <= 255, "set_color: the green component must be >= 0 and <= 255");
	assert_lua_error(L, b >= 0 && b <= 255, "set_color: the blue component must be >= 0 and <= 255");

	Engine &engine = get_engine();
	engine.display.set_color(r, g, b);
	return 0;
}

int mlua_set_alpha(lua_State* L)
{
	assert(L);

	int alpha = luaL_checkint(L, 1);

	assert_lua_error(L, alpha >= 0 && alpha <= 255, "set_alpha: the alpha component must be >= 0 and <= 255");

	Engine &engine = get_engine();
	engine.display.set_alpha(alpha);
	return 0;
}

int mlua_set_point_size(lua_State* L)
{
	assert(L);

	lua_Number point_size = luaL_checknumber(L, 1);

	assert_lua_error(L, point_size >= 0, "set_point_size: must be >= 0");

	Engine &engine = get_engine();
	engine.display.set_point_size(point_size);
	return 0;
}

int mlua_set_line_width(lua_State* L)
{
	assert(L);

	lua_Number width = luaL_checknumber(L, 1);

	assert_lua_error(L, width >= 0, "set_line_width: must be >= 0");

	Engine &engine = get_engine();
	engine.display.set_line_width(width);
	return 0;
}

int mlua_set_fullscreen(lua_State* L)
{
	assert(L);

	bool fullscreen = lua_toboolean(L, 1);
	Engine &engine = get_engine();
	engine.display.set_fullscreen(fullscreen);

	push_surface(L, engine.display.get_screen());
	lua_setfield(L, LUA_REGISTRYINDEX, "screen");
	return 0;
}

int mlua_set_title(lua_State* L)
{
	assert(L);

	const char *title = luaL_checkstring(L, 1);
	Engine &engine = get_engine();
	engine.display.set_title(title);
	return 0;
}

int mlua_set_blend_mode(lua_State* L)
{
	assert(L);

	BlendMode mode = static_cast<BlendMode>(luaL_checknumber(L, 1));
	Engine &engine = get_engine();
	engine.display.set_blend_mode(mode);
	return 0;
}

int mlua_show_cursor(lua_State* L)
{
	assert(L);

	bool show = lua_toboolean(L, 1);
	Engine &engine = get_engine();
	engine.display.show_cursor(show);
	return 0;
}

int mlua_resize(lua_State* L)
{
	assert(L);

	int w = luaL_checkint(L, 1);
	int h = luaL_checkint(L, 2);

	assert_lua_error(L, w > 0, "resize: width must be > 0");
	assert_lua_error(L, h > 0, "resize: height must be > 0");

	Engine &engine = get_engine();
	engine.display.resize(w, h);

	// make sure we don't free the screen until the next resize
	push_surface(L, engine.display.get_screen());
	lua_setfield(L, LUA_REGISTRYINDEX, "screen");
	return 0;
}

int mlua_screen2scene(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	lua_Number x = luaL_checknumber(L, 1);
	lua_Number y = luaL_checknumber(L, 2);
	float tx, ty;
	engine.display.screen2scene(x, y, &tx, &ty);
	lua_pushnumber(L, tx);
	lua_pushnumber(L, ty);
	return 2;
}

int mlua_surface_class_index(lua_State* L)
{
	assert(L);

	Surface* surface = pop_surface(L, 1);
	const char* index = luaL_checkstring(L, 2);
	if (strcmp(index, "w") == 0) {
		lua_pushnumber(L, surface->w);
	} else if (strcmp(index, "h") == 0) {
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
	Engine &engine = get_engine();
	const char * filename = luaL_checkstring(L, 1);
	r = engine.display.load_surface(filename, &surface);
	assert_lua_error(L, r != -E2BIG, "load_surface: surface size must be width > 0 and <= 2048, height > 0 and <= 2048");
	if (r < 0) {
		return luaL_fileresult(L, 0, filename);
	}
	push_surface(L, surface);
	return 1;
}

int mlua_new_surface(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	int w = luaL_checkint(L, 1);
	int h = luaL_checkint(L, 2);

	assert_lua_error(L, w > 0 && w <= 2048, "new_surface: width must be > 0 and <= 2048");
	assert_lua_error(L, h > 0 && h <= 2048, "new_surface: width must be > 0 and <= 2048");

	bool force_npot = lua_toboolean(L, 3);
	Surface* surface = engine.display.new_surface(w, h, force_npot);
	push_surface(L, surface);
	return 1;
}

int mlua_free_surface(lua_State* L)
{
	assert(L);

	log_debug("");
	Engine &engine = get_engine();
	Surface* surface = pop_surface(L, 1);
	engine.display.free_surface(surface);
	return 0;
}

int mlua_draw_on_surface(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Surface* old = engine.display.get_draw_on();
	Surface* surface = pop_surface(L, 1);
	engine.display.draw_on(surface);

	if (old) {
		push_surface(L, old);
		return 1;
	}
	return 0;
}

int mlua_draw_from_surface(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Surface* old = engine.display.get_draw_from();
	Surface* surface = pop_surface(L, 1);
	engine.display.draw_from(surface);

	if (old) {
		push_surface(L, old);
		return 1;
	}
	return 0;
}

int mlua_set_filter_surface(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Surface* surface = pop_surface(L, 1);
	FilterMode mode = static_cast<FilterMode>(luaL_checknumber(L, 2));
	engine.display.set_filter(surface, mode);
	return 0;
}

int mlua_draw_background(lua_State*)
{
	Engine &engine = get_engine();
	engine.display.draw_background();
	return 0;
}

int mlua_draw_point(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Buffer* buffer = engine.display.get_current_buffer();
	if (buffer->is_user_buffer()) {
		BufferType type = buffer->get_type();
		if (type != UNDEFINED && (type != POINT_BUFFER || buffer->has_texture()))
			return luaL_error(L, "the current buffer cannot contain points");
		if (buffer->is_full())
			return luaL_error(L, "the current buffer is full");
	}

	lua_Number x = luaL_checknumber(L, 1);
	lua_Number y = luaL_checknumber(L, 2);
	engine.display.draw_point(x, y);
	return 0;
}

int mlua_draw_point_tex(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Buffer* buffer = engine.display.get_current_buffer();
	if (buffer->is_user_buffer()) {
		BufferType type = buffer->get_type();
		if (engine.display.is_debug() && (type == UNDEFINED || type == LINE_BUFFER)) {
		} else if (type != UNDEFINED && (type != POINT_BUFFER || !buffer->has_texture()))
			return luaL_error(L, "the current buffer cannot contain points");
		if (buffer->is_full())
			return luaL_error(L, "the current buffer is full");
	}

	lua_Number xi = luaL_checknumber(L, 1);
	lua_Number yi = luaL_checknumber(L, 2);
	lua_Number xd = luaL_checknumber(L, 3);
	lua_Number yd = luaL_checknumber(L, 4);
	engine.display.draw_point_tex(xi, yi, xd, yd);
	return 0;
}

int mlua_draw_line(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Buffer* buffer = engine.display.get_current_buffer();
	if (buffer->is_user_buffer()) {
		BufferType type = buffer->get_type();
		if (type != UNDEFINED && (type != LINE_BUFFER || buffer->has_texture()))
			return luaL_error(L, "the current buffer cannot contain lines");
		if (buffer->is_full())
			return luaL_error(L, "the current buffer is full");
	}

	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);
	engine.display.draw_line(x1, y1, x2, y2);
	return 0;
}

int mlua_draw_triangle(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Buffer* buffer = engine.display.get_current_buffer();
	if (buffer->is_user_buffer()) {
		BufferType type = buffer->get_type();
		if (engine.display.is_debug() && (type == UNDEFINED || type == LINE_BUFFER)) {
		} else if (type != UNDEFINED && (type != TRIANGLE_BUFFER || buffer->has_texture()))
			return luaL_error(L, "the current buffer cannot contain triangles");
		if (buffer->is_full())
			return luaL_error(L, "the current buffer is full");
	}

	lua_Number x1 = luaL_checknumber(L, 1);
	lua_Number y1 = luaL_checknumber(L, 2);
	lua_Number x2 = luaL_checknumber(L, 3);
	lua_Number y2 = luaL_checknumber(L, 4);
	lua_Number x3 = luaL_checknumber(L, 5);
	lua_Number y3 = luaL_checknumber(L, 6);
	engine.display.draw_triangle(x1, y1, x2, y2, x3, y3);
	return 0;
}

int mlua_draw_surface(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Buffer* buffer = engine.display.get_current_buffer();
	if (buffer->is_user_buffer()) {
		BufferType type = buffer->get_type();
		if (engine.display.is_debug() && (type == UNDEFINED || type == LINE_BUFFER)) {
		} else if (type != UNDEFINED && (type != TRIANGLE_BUFFER || !buffer->has_texture()))
			return luaL_error(L, "the current buffer cannot contain textured triangles");
		if (buffer->is_full())
			return luaL_error(L, "the current buffer is full");
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
	engine.display.draw_surface(i1, i2, i3, i4, i5, i6,
	                            o1, o2, o3, o4, o5, o6);
	return 0;
}

int mlua_draw_quad(lua_State* L)
{
	assert(L);

	Engine &engine = get_engine();
	Buffer* buffer = engine.display.get_current_buffer();
	if (buffer->is_user_buffer()) {
		BufferType type = buffer->get_type();
		if (engine.display.is_debug() && (type == UNDEFINED || type == LINE_BUFFER)) {
		} else if (type != UNDEFINED && (type != TRIANGLE_BUFFER || !buffer->has_texture()))
			return luaL_error(L, "the current buffer cannot contain textured triangles");
		if (buffer->is_full(2))
			return luaL_error(L, "the current buffer is full");
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
	engine.display.draw_quad(i1, i2, i3, i4, i5, i6, i7, i8,
	                         o1, o2, o3, o4, o5, o6, o7, o8);
	return 0;
}

