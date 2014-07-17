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
#include <lua.hpp>

#include "engine.hpp"
#include "lua_functions.hpp"
#include "font.hpp"

DECLARE_PUSHPOP(Font, font)

int mlua_draw_font(lua_State* L)
{
	Font* font = pop_font(L, 1);
	const char* text = luaL_checkstring(L, 2);
	lua_Number x = luaL_checknumber(L, 3);
	lua_Number y = luaL_checknumber(L, 4);
	font->draw(text, x, y);
	return 0;
}

int mlua_draw_plain_font(lua_State* L)
{
	Font* font = pop_font(L, 1);
	const char* text = luaL_checkstring(L, 2);
	lua_Number x = luaL_checknumber(L, 3);
	lua_Number y = luaL_checknumber(L, 4);
	font->draw_plain(text, x, y);
	return 0;
}

int mlua_load_font(lua_State* L)
{
	const char* filename = luaL_checkstring(L, 1);
	lua_Number size = luaL_checknumber(L, 2);
	Font* font = Font::load(filename, size);
	if (font) {
		push_font(L, font);
		return 1;
	}
	return luaL_fileresult(L, 0, filename);
}

int mlua_sizeof_font(lua_State* L)
{
	Font* font = pop_font(L, 1);
	const char* text = luaL_checkstring(L, 2);
	lua_Number w, h;
	font->get_textsize(text, &w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}

int mlua_sizeof_plain_font(lua_State* L)
{
	Font* font = pop_font(L, 1);
	const char* text = luaL_checkstring(L, 2);
	lua_Number w, h;
	font->get_textsize_plain(text, &w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}

int mlua_free_font(lua_State* L)
{
	Font* font = pop_font(L, 1);
	delete font;
	return 0;
}

BEGIN_MODULE(truetype)
	DECLARE_FUNCTION(load_font)

	BEGIN_CLASS(font)
		ADD_METHOD(font, draw)
		ADD_METHOD(font, draw_plain)
		ADD_METHOD(font, sizeof)
		ADD_METHOD(font, sizeof_plain)
		ADD_GC(free_font)
		END_CLASS();
	REGISTER_CLASS(font, "Font");
END_MODULE()

