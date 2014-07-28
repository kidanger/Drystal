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
#pragma once

struct lua_State;

int mlua_set_color(lua_State* L);
int mlua_set_alpha(lua_State* L);
int mlua_set_point_size(lua_State* L);
int mlua_set_line_width(lua_State* L);
int mlua_set_title(lua_State* L);
int mlua_set_blend_mode(lua_State* L);

int mlua_camera__newindex(lua_State* L);
int mlua_camera__index(lua_State* L);
int mlua_camera_reset(lua_State*);

int mlua_show_cursor(lua_State* L);
int mlua_resize(lua_State* L);
int mlua_set_fullscreen(lua_State* L);
int mlua_screen2scene(lua_State* L);

int mlua_surface_class_index(lua_State* L);
int mlua_load_surface(lua_State* L);
int mlua_new_surface(lua_State* L);
int mlua_free_surface(lua_State* L);
int mlua_draw_on_surface(lua_State* L);
int mlua_draw_from_surface(lua_State* L);
int mlua_set_filter_surface(lua_State* L);

int mlua_draw_background(lua_State*);
int mlua_draw_point(lua_State* L);
int mlua_draw_point_tex(lua_State* L);
int mlua_draw_line(lua_State* L);
int mlua_draw_triangle(lua_State* L);
int mlua_draw_surface(lua_State* L);
int mlua_draw_quad(lua_State* L);

int mlua_new_shader(lua_State* L);
int mlua_use_shader(lua_State* L);
int mlua_feed_shader(lua_State* L);
int mlua_free_shader(lua_State* L);

