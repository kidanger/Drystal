#define LUA_API extern

#include <lua.hpp>
#include <SDL/SDL_opengl.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "engine.hpp"

unsigned char file_content[1<<20];
unsigned char pixels[512*512];
unsigned char pixels_colored[512*512*4];

struct Font {
	Surface* surface;
	float size;
	int first_char;
	int num_chars;
	stbtt_bakedchar* char_data;
};

static Font* current_font;

Font* load_font(const char* filename, float size, int first_char=32, int num_chars=96)
{
	Engine& engine = get_engine();

	FILE* file = fopen(filename, "rb");
	if (not file)
		return nullptr;

	// TODO: compute texture size
	Surface* surf = engine.display.new_surface(512, 512);

	Font* font = new Font;
	font->surface = surf;

	font->first_char = first_char;
	font->num_chars = num_chars;
	font->char_data = new stbtt_bakedchar[num_chars];
	font->size = size;

	assert(fread(file_content, 1, 1<<20, file));
	fclose(file);

	stbtt_BakeFontBitmap(file_content, 0, size, pixels, surf->w, surf->h,
						first_char, num_chars, font->char_data);

	memset(pixels_colored, 0xff, 512*512*4);
	for (int i = 0; i < 512*512; i++) {
		pixels_colored[i*4+3] = pixels[i];
	}
	// FIXME: don't overwrite texture, let display.cpp do this
	glBindTexture(GL_TEXTURE_2D, surf->tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels_colored);
	GLDEBUG();

	return font;
}

void draw_text(const unsigned char* text, float x, float y)
{
	assert(current_font);
	int start = current_font->first_char;
	int end = start + current_font->num_chars;
	y += current_font->size;

	Engine& engine = get_engine();
	engine.display.draw_from(current_font->surface);
	while (*text) {
		if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(current_font->char_data, *text - start, &x, &y, &q);
			engine.display.draw_surface(
					// texture coordinates
					q.s0, q.t0,
					q.s1, q.t0,
					q.s1, q.t1,
					q.s0, q.t1,
					// screen coordinates
					q.x0, q.y0,
					q.x1, q.y0,
					q.x1, q.y1,
					q.x0, q.y1
			);
		}
		++text;
	}
}

void text_size(const unsigned char* text, int* w, int* h)
{
	assert(current_font);
	float x = 0, y = 0;
	int start = current_font->first_char;
	int end = start + current_font->num_chars;
	while (*text) {
		if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(current_font->char_data, *text - start, &x, &y, &q);
		}
		++text;
	}
	*w = x;
	*h = y;
}

void use_font(Font* font)
{
	current_font = font;
}

void free_font(Font* font)
{
	if (font == current_font)
		current_font = nullptr;

	Engine& engine = get_engine();
	engine.display.free_surface(font->surface);
	delete[] font->char_data;
}

int draw_text_wrap(lua_State* L)
{
	const char* text = lua_tostring(L, 1);
	lua_Number x = lua_tonumber(L, 2);
	lua_Number y = lua_tonumber(L, 3);
	draw_text((const unsigned char*)text, x, y);
	return 0;
}

int load_font_wrap(lua_State* L)
{
	const char* filename = lua_tostring(L, 1);
	lua_Number size = lua_tonumber(L, 2);
	Font* font = load_font(filename, size);
	if (font) {
		lua_pushlightuserdata(L, font);
		return 1;
	}
	return 0;
}

int use_font_wrap(lua_State* L)
{
	Font* font = (Font*) lua_touserdata(L, 1);
	use_font(font);
	return 0;
}

int text_size_wrap(lua_State* L)
{
	const char* text = lua_tostring(L, 1);
	int w, h;
	text_size((const unsigned char*)text, &w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}

int free_font_wrap(lua_State* L)
{
	Font* font = (Font*) lua_touserdata(L, 1);
	free_font(font);
	return 0;
}

static const luaL_Reg lib[] =
{
	{"draw", draw_text_wrap},
	{"load", load_font_wrap},
	{"use", use_font_wrap},
	{"sizeof", text_size_wrap},
	{"free", free_font_wrap},
	{NULL, NULL}
};

LUA_API "C" int luaopen_truetype(lua_State *L)
{
    luaL_newlibtable(L, lib);
    luaL_setfuncs(L, lib, 0);
	return 1;
}

