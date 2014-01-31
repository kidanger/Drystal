#include <cassert>

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
#include "font.hpp"


// TODO: fixme
unsigned char file_content[1<<20];
unsigned char pixels[512*512];
unsigned char pixels_colored[512*512*4];

Font* Font::load(const char* filename, float size, int first_char, int num_chars)
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
	font->font_size = size;

	size_t read = fread(file_content, 1, 1<<20, file);
	assert(read);
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

Font::~Font()
{
	Engine& engine = get_engine();
	engine.display.free_surface(surface);
	delete[] char_data;
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
static inline void draw_quad_fancy(Engine& engine, const stbtt_aligned_quad& q,
		float italic=0.0, float dx=0.0, float dy=0.0)
{
	engine.display.draw_quad(
			// texture coordinates
			q.s0, q.t0,
			q.s1, q.t0,
			q.s1, q.t1,
			q.s0, q.t1,
			// screen coordinates
			q.x0 + italic + dx, q.y0 + dy,
			q.x1 + italic + dx, q.y0 + dy,
			q.x1 + dx, q.y1 + dy,
			q.x0 + dx, q.y1 + dy
	);
}

void Font::draw_plain(const char* text, float x, float y)
{
	int start = this->first_char;
	int end = start + num_chars;
	y += font_size * 3 / 4;

	Engine& engine = get_engine();
	const Surface* old_surface = engine.display.get_draw_from();
	engine.display.draw_from(surface);
	while (*text) {
		if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(char_data, *text - start, &x, &y, &q);
			draw_quad(engine, q);
		}
		++text;
	}
	if (old_surface)
		engine.display.draw_from(old_surface);
}

void Font::draw(const char* text, float x, float y)
{
	int start = first_char;
	int end = start + num_chars;
	y += font_size * 3 / 4;

	Engine& engine = get_engine();
	const Surface* old_surface = engine.display.get_draw_from();
	engine.display.draw_from(surface);
	while (*text) {
		if (*text == '{') {
			float size = 1.0;
			float italic = 0.0;
			int r, g, b;
			int alpha;
			bool outlined = false;
			int outr = 0, outg = 0, outb = 0;
			bool shadow = false;
			float shadow_x = 0, shadow_y = 0;
			engine.display.get_color(&r, &g, &b);
			engine.display.get_alpha(&alpha);
			int oldr = r, oldg = g, oldb = b;
			unsigned char oldalpha = alpha;
			text++;
token:
			const char * start_text = text;
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
				else if (!strncmp((const char*) start_text, (const char*) "italic", text - start_text))
					italic = 3.5;
				else if (!strncmp((const char*) start_text, (const char*) "outline", text - start_text))
					outlined = true;
				else if (!strncmp((const char*) start_text, (const char*) "outr", 4))
					outr = atoi((const char*) start_text + 4);
				else if (!strncmp((const char*) start_text, (const char*) "outg", 4))
					outg = atoi((const char*) start_text + 4);
				else if (!strncmp((const char*) start_text, (const char*) "outb", 4))
					outb = atoi((const char*) start_text + 4);
				else if (!strncmp((const char*) start_text, (const char*) "shadowx", 7)) {
					shadow_x = atof((const char*) start_text + 7);
					shadow = true;
				} else if (!strncmp((const char*) start_text, (const char*) "shadowy", 7)) {
					shadow_y = atof((const char*) start_text + 7);
					shadow = true;
				} else {
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
						stbtt_GetBakedQuad(char_data, chr - start, &x, &y, &q, size);
						if (shadow) {
							engine.display.set_color(0, 0, 0);
							draw_quad_fancy(engine, q, italic,  shadow_x, shadow_y);
							engine.display.set_color(r, g, b);
						}
						if (outlined) {
							engine.display.set_color(outr, outg, outb);
							float f = font_size * 0.04;
							draw_quad_fancy(engine, q, italic,       -1*f,        0*f);
							draw_quad_fancy(engine, q, italic,        1*f,        0*f);
							draw_quad_fancy(engine, q, italic,        0*f,       -1*f);
							draw_quad_fancy(engine, q, italic,        0*f,        1*f);
							draw_quad_fancy(engine, q, italic,  M_SQRT1_2*f,  M_SQRT1_2*f);
							draw_quad_fancy(engine, q, italic, -M_SQRT1_2*f,  M_SQRT1_2*f);
							draw_quad_fancy(engine, q, italic, -M_SQRT1_2*f, -M_SQRT1_2*f);
							draw_quad_fancy(engine, q, italic,  M_SQRT1_2*f, -M_SQRT1_2*f);
							engine.display.set_color(r, g, b);
						}
						draw_quad_fancy(engine, q, italic);
					}
					i++;
				}
				engine.display.set_color(oldr, oldg, oldb);
				engine.display.set_alpha(oldalpha);
				x += italic;
			}
		}
		else if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(char_data, *text - start, &x, &y, &q);
			draw_quad(engine, q);
		}
		text++;
	}
	if (old_surface)
		engine.display.draw_from(old_surface);
}

void Font::get_textsize_plain(const char* text, float* w, float* h)
{
	float x = 0, y = 0;
	int maxy = 0;
	int maxx = 0;
	int start = first_char;
	int end = start + num_chars;
	while (*text) {
		if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(char_data, *text - start, &x, &y, &q);
			maxy = q.y1 - q.y0 > maxy ? q.y1 - q.y0 : maxy;
			maxx = q.x1;
		}
		text++;
	}
	*w = maxx;
	*h = maxy;
}

void Font::get_textsize(const char* text, float* w, float* h)
{
	float x = 0, y = 0;
	int maxy = 0;
	int maxx = 0;
	int start = first_char;
	int end = start + num_chars;
	while (*text) {
		if (*text == '{') {
			float size = 1.0;
			float italic = 0.0;
			text++;
token:
			const char * start_text = text; //(const unsigned char*) text;
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
				else if (!strncmp((const char*) start_text, (const char*) "italic", text - start_text))
					italic = 3.5;
				text++;
				goto token;
			} else {
				// end of command, start_text -> text is the text to render
				int i = 0;
				while (start_text + i < text) {
					unsigned char chr = *(start_text + i);
					if (chr >= start && chr < end) {
						stbtt_aligned_quad q;
						stbtt_GetBakedQuad(char_data, chr - start, &x, &y, &q, size);
						maxy = q.y1 - q.y0 > maxy ? q.y1 - q.y0 : maxy;
						maxx = q.x1;
					}
					i++;
					x += italic;
				}
			}
		}
		else if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(char_data, *text - start, &x, &y, &q);
			maxy = q.y1 - q.y0 > maxy ? q.y1 - q.y0 : maxy;
			maxx = q.x1;
		}
		text++;
	}
	*w = maxx;
	*h = maxy;
}

