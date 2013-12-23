#include <lua.hpp>
#ifndef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif

extern "C" {
#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
}

#include "engine.hpp"
#include "lua_functions.hpp"

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

DECLARE_PUSHPOP(Font, font)

Font* load_font(const char* filename, float size, int first_char=32, int num_chars=96)
{
	Engine& engine = get_engine();

	FILE* file = fopen(filename, "rb");
	if (not file)
		return NULL;

	// TODO: compute texture size
	int w = 512;
	int h = 512;
	Font* font = new Font;

	font->first_char = first_char;
	font->num_chars = num_chars;
	font->char_data = new stbtt_bakedchar[num_chars];
	font->size = size;

	assert(fread(file_content, 1, 1<<20, file));
	fclose(file);

	stbtt_BakeFontBitmap(file_content, 0, size, pixels, w, h,
						first_char, num_chars, font->char_data);

	memset(pixels_colored, 0xff, w*h*4);
	for (int i = 0; i < w*h; i++) {
		pixels_colored[i*4+3] = pixels[i];
	}

	font->surface = engine.display.create_surface(w, h, w, h, pixels_colored);

	return font;
}

static inline void draw_quad(Engine& engine, const stbtt_aligned_quad& q)
{
	engine.display.draw_quad(
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

void draw_text(const unsigned char* text, float x, float y)
{
	assert(current_font);
	int start = current_font->first_char;
	int end = start + current_font->num_chars;
	y += current_font->size * 3 / 4;

	Engine& engine = get_engine();
	const Surface* old_surface = engine.display.get_draw_from();
	engine.display.draw_from(current_font->surface);
	while (*text) {
		if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(current_font->char_data, *text - start, &x, &y, &q);
			draw_quad(engine, q);
		}
		++text;
	}
	if (old_surface)
		engine.display.draw_from(old_surface);
}

void draw_text_color(const unsigned char* text, float x, float y)
{
	assert(current_font);
	int start = current_font->first_char;
	int end = start + current_font->num_chars;
	y += current_font->size * 3 / 4;

	Engine& engine = get_engine();
	const Surface* old_surface = engine.display.get_draw_from();
	engine.display.draw_from(current_font->surface);
	while (*text) {
		if (*text == '{') {
			float size = 1.0;
			int r, g, b;
			int alpha;
			engine.display.get_color(&r, &g, &b);
			engine.display.get_alpha(&alpha);
			int oldr = r, oldg = g, oldb = b;
			unsigned char oldalpha = alpha;
			text++;
token:
			const unsigned char * start_text = text;
			while (*text != '|' and *text != '}') {
				text++;
			}
			if (*text == '|') {
				if (*(text - 1) == '%')
					alpha = atoi((const char*) start_text) / 100. * 255;
				else if (*start_text == 'r')
					r = atoi((const char*) start_text + 1);
				else if (*start_text == 'g')
					g = atoi((const char*) start_text + 1);
				else if (*start_text == 'b' and *(start_text+1) != 'i') // ignore 'big'
					b = atoi((const char*) start_text + 1);
				else if (!strncmp((const char*) start_text, (const char*) "big", text - start_text))
					size = 1.3;
				else if (!strncmp((const char*) start_text, (const char*) "BIG", text - start_text))
					size = 1.7;
				else if (!strncmp((const char*) start_text, (const char*) "small", text - start_text))
					size = 0.8;
				else if (!strncmp((const char*) start_text, (const char*) "tiny", text - start_text))
					size = 0.6;
				else {
					printf("truetype.draw_color: unknown command %.*s\n", (int)(text - start_text), start_text);
				}
				text++;
				goto token;
			} else {
				// end of command, start_text -> text is the text to render
				engine.display.set_color(r, g, b);
				engine.display.set_alpha(alpha);
				int i = 0;
				while (start_text + i < text) {
					unsigned char chr = *(start_text + i);
					if (chr >= start && chr < end) {
						stbtt_aligned_quad q;
						stbtt_GetBakedQuad(current_font->char_data, chr - start, &x, &y, &q, size);
						draw_quad(engine, q);
					}
					i++;
				}
				engine.display.set_color(oldr, oldg, oldb);
				engine.display.set_alpha(oldalpha);
			}
		}
		else if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(current_font->char_data, *text - start, &x, &y, &q);
			draw_quad(engine, q);
		}
		text++;
	}
	if (old_surface)
		engine.display.draw_from(old_surface);
}

void _sizeof(const unsigned char* text, int* w, int* h)
{
	assert(current_font);
	float x = 0, y = 0;
	int maxy = 0;
	int maxx = 0;
	int start = current_font->first_char;
	int end = start + current_font->num_chars;
	while (*text) {
		if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(current_font->char_data, *text - start, &x, &y, &q);
			maxy = q.y1 - q.y0 > maxy ? q.y1 - q.y0 : maxy;
			maxx = q.x1;
		}
		text++;
	}
	*w = maxx;
	*h = maxy;
}

void sizeof_color(const unsigned char* text, int* w, int* h)
{
	assert(current_font);
	float x = 0, y = 0;
	int maxy = 0;
	int maxx = 0;
	int start = current_font->first_char;
	int end = start + current_font->num_chars;
	while (*text) {
		if (*text == '{') {
			float size = 1.0;
			text++;
token:
			const unsigned char * start_text = text;
			while (*text != '|' and *text != '}') {
				text++;
			}
			if (*text == '|') {
				if (!strncmp((const char*) start_text, (const char*) "big", text - start_text))
					size = 1.3;
				else if (!strncmp((const char*) start_text, (const char*) "BIG", text - start_text))
					size = 1.7;
				else if (!strncmp((const char*) start_text, (const char*) "small", text - start_text))
					size = 0.8;
				else if (!strncmp((const char*) start_text, (const char*) "tiny", text - start_text))
					size = 0.6;
				text++;
				goto token;
			} else {
				// end of command, start_text -> text is the text to render
				int i = 0;
				while (start_text + i < text) {
					unsigned char chr = *(start_text + i);
					if (chr >= start && chr < end) {
						stbtt_aligned_quad q;
						stbtt_GetBakedQuad(current_font->char_data, chr - start, &x, &y, &q, size);
						maxy = q.y1 - q.y0 > maxy ? q.y1 - q.y0 : maxy;
						maxx = q.x1;
					}
					i++;
				}
			}
		}
		else if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(current_font->char_data, *text - start, &x, &y, &q);
			maxy = q.y1 - q.y0 > maxy ? q.y1 - q.y0 : maxy;
			maxx = q.x1;
		}
		text++;
	}
	*w = maxx;
	*h = maxy;
}

void use_font(Font* font)
{
	current_font = font;
}

void free_font(Font* font)
{
	if (font == current_font)
		current_font = NULL;

	Engine& engine = get_engine();
	engine.display.free_surface(font->surface);
	delete[] font->char_data;
}

int draw_text_wrap(lua_State* L)
{
	const char* text = luaL_checkstring(L, 1);
	lua_Number x = lua_tonumber(L, 2);
	lua_Number y = lua_tonumber(L, 3);
	draw_text((const unsigned char*)text, x, y);
	return 0;
}
int draw_text_color_wrap(lua_State* L)
{
	const char* text = luaL_checkstring(L, 1);
	lua_Number x = lua_tonumber(L, 2);
	lua_Number y = lua_tonumber(L, 3);
	draw_text_color((const unsigned char*)text, x, y);
	return 0;
}

int load_font_wrap(lua_State* L)
{
	const char* filename = luaL_checkstring(L, 1);
	lua_Number size = lua_tonumber(L, 2);
	Font* font = load_font(filename, size);
	if (font) {
        push_font(L, font);
		return 1;
	}
	return 0;
}

int use_font_wrap(lua_State* L)
{
	Font* font = pop_font(L, 1);
	use_font(font);
	return 0;
}

int sizeof_wrap(lua_State* L)
{
	const char* text = luaL_checkstring(L, 1);
	int w, h;
	_sizeof((const unsigned char*)text, &w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}

int sizeof_color_wrap(lua_State* L)
{
	const char* text = luaL_checkstring(L, 1);
	int w, h;
	sizeof_color((const unsigned char*)text, &w, &h);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 2;
}

int free_font_wrap(lua_State* L)
{
	Font* font = pop_font(L, 1);
	free_font(font);
	return 0;
}

static const luaL_Reg lib[] =
{
	{"draw", draw_text_wrap},
	{"draw_color", draw_text_color_wrap},
	{"load", load_font_wrap},
	{"use", use_font_wrap},
	{"sizeof", sizeof_wrap},
	{"sizeof_color", sizeof_color_wrap},
	{"free", free_font_wrap},
	{NULL, NULL}
};

DEFINE_EXTENSION(truetype)
{
    DECLARE_GC(font, free_font_wrap)
    REGISTER_GC(font);
	luaL_newlibtable(L, lib);
	luaL_setfuncs(L, lib, 0);
	return 1;
}

