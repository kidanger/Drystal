#include <lua.hpp>

#include "engine.hpp"
#include "lua_functions.hpp"
#include "font.hpp"

static Font* current_font;

DECLARE_PUSHPOP(Font, font)

int mlua_draw_font(lua_State* L)
{
	const char* text = luaL_checkstring(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	current_font->draw(text, x, y);
	return 0;
}

int mlua_draw_plain_font(lua_State* L)
{
	const char* text = luaL_checkstring(L, 1);
	lua_Number x = luaL_checknumber(L, 2);
	lua_Number y = luaL_checknumber(L, 3);
	current_font->draw_plain(text, x, y);
	return 0;
}

int load_font_wrap(lua_State* L)
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

int mlua_use_font(lua_State* L)
{
	Font* font = pop_font(L, 1);
	current_font = font;
	return 0;
}

int mlua_sizeof_font(lua_State* L)
{
	const char* text = luaL_checkstring(L, 1);
	lua_Number w, h;
	current_font->get_textsize(text, &w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}

int mlua_sizeof_plain_font(lua_State* L)
{
	const char* text = luaL_checkstring(L, 1);
	lua_Number w, h;
	current_font->get_textsize_plain(text, &w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}

int mlua_free_font(lua_State* L)
{
	Font* font = pop_font(L, 1);
	if (current_font == font)
		current_font = NULL;
	delete font;
	return 0;
}

static const luaL_Reg lib[] =
{
	{"load", load_font_wrap},
	ADD_METHOD(font, draw)
	ADD_METHOD(font, draw_plain)
	ADD_METHOD(font, use)
	{"sizeof", mlua_sizeof_font},
	ADD_METHOD(font, sizeof_plain)
	{NULL, NULL}
};

DEFINE_EXTENSION(truetype)
{
	luaL_newlib(L, lib);

	BEGIN_CLASS(font)
		ADD_GC(free_font)
		END_CLASS();
	REGISTER_CLASS(font, "__Font");

	return 1;
}

