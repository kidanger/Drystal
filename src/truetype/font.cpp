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
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cmath>

#ifndef EMSCRIPTEN
#include <SDL2/SDL_opengles2.h>
#else
#include <SDL/SDL_opengl.h>
#endif

#include <stb_truetype.h>

#include "engine.hpp"
#include "macro.h"
#include "font.hpp"
#include "parser.hpp"


// TODO: fixme
unsigned char file_content[1 << 20];
unsigned char pixels[512 * 512];
unsigned char pixels_colored[512 * 512 * 4];

Font* Font::load(const char* filename, float size, int first_char, int num_chars)
{
	assert(filename);

	Engine& engine = get_engine();

	FILE* file = fopen(filename, "rb");
	if (!file)
		return NULL;

	// TODO: compute texture size
	int w = 512;
	int h = 512;
	Font* font = new Font;

	font->first_char = first_char;
	font->num_chars = num_chars;
	font->char_data = new stbtt_bakedchar[num_chars];
	font->font_size = size;

	size_t read = fread(file_content, 1, 1 << 20, file);
	if (read == 0) {
		fclose(file);
		return NULL;
	}
	fclose(file);

	stbtt_BakeFontBitmap(file_content, 0, size, pixels, w, h,
	                     first_char, num_chars, font->char_data);

	memset(pixels_colored, 0xff, w * h * 4);
	for (int i = 0; i < w * h; i++) {
		pixels_colored[i * 4 + 3] = pixels[i];
	}

	font->surface = engine.display.create_surface(w, h, w, h, pixels_colored);
	engine.display.set_filter(font->surface, NEAREST);
	font->ref = 0;

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
                                   float italic = 0.0, float dx = 0.0, float dy = 0.0)
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
	assert(text);

	int initialx = x;
	int start = first_char;
	int end = start + num_chars;
	y += font_size * 3 / 4;

	Engine& engine = get_engine();
	Surface* old_surface = engine.display.get_draw_from();
	engine.display.draw_from(surface);
	while (*text) {
		if (*text == '\n') {
			x = initialx;
			y += font_size;
		} else if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(char_data, *text - start, &x, &y, &q, 1.0f);
			draw_quad(engine, q);
		}
		++text;
	}
	if (old_surface)
		engine.display.draw_from(old_surface);
}

void Font::draw(const char* text, float x, float y, Alignment align)
{
	assert(text);

	int initialx = x;
	int start = first_char;
	int end = start + num_chars;
	y += font_size * 3 / 4;

	Engine& engine = get_engine();
	Surface* old_surface = engine.display.get_draw_from();
	engine.display.draw_from(surface);

	const char* textend = text;
	int r, g, b, a;
	engine.display.get_color(&r, &g, &b);
	engine.display.get_alpha(&a);

	TextState* state = push_parser();
	if (!state) {
		return;
	}
	state->r = r;
	state->g = g;
	state->b = b;
	state->alpha = a;
	while (parse(&state, text, textend)) {
		engine.display.set_color(state->r, state->g, state->b);
		engine.display.set_alpha(state->alpha);

		float line_width;
		float line_height;
		if (*(text-1) != '}' && *(text-1) != '|') {
			get_textsize(text, &line_width, &line_height, 1);
			if (align == ALIGN_CENTER) {
				x = initialx - line_width / 2;
			} else if (align == ALIGN_RIGHT) {
				x = initialx - line_width;
			}
		}

		while (text < textend) {
			unsigned char chr = *text;
			if (chr == '\n') {
				get_textsize(text + 1, &line_width, &line_height, 1);

				if (align == ALIGN_LEFT) {
					x = initialx;
				} else if (align == ALIGN_CENTER) {
					x = initialx - line_width / 2;
				} else if (align == ALIGN_RIGHT) {
					x = initialx - line_width;
				}
				y += line_height;
			} else if (chr >= start && chr < end) {
				float italic = state->italic;
				stbtt_aligned_quad q;
				stbtt_GetBakedQuad(char_data, chr - start, &x, &y, &q, state->size);
				if (state->shadow) {
					engine.display.set_color(0, 0, 0);
					draw_quad_fancy(engine, q, italic, state->shadow_x, state->shadow_y);
					engine.display.set_color(state->r, state->g, state->b);
				}
				if (state->outlined) {
					engine.display.set_color(state->outr, state->outg, state->outb);
					float f = font_size * 0.04;
					draw_quad_fancy(engine, q, italic,       -1 * f,        0 * f);
					draw_quad_fancy(engine, q, italic,        1 * f,        0 * f);
					draw_quad_fancy(engine, q, italic,        0 * f,       -1 * f);
					draw_quad_fancy(engine, q, italic,        0 * f,        1 * f);
					draw_quad_fancy(engine, q, italic,  M_SQRT1_2 * f,  M_SQRT1_2 * f);
					draw_quad_fancy(engine, q, italic, -M_SQRT1_2 * f,  M_SQRT1_2 * f);
					draw_quad_fancy(engine, q, italic, -M_SQRT1_2 * f, -M_SQRT1_2 * f);
					draw_quad_fancy(engine, q, italic,  M_SQRT1_2 * f, -M_SQRT1_2 * f);
					engine.display.set_color(state->r, state->g, state->b);
				}
				draw_quad_fancy(engine, q, italic);
			}
			text++;
		}
	}
	pop_parser();
	engine.display.set_color(r, g, b);
	engine.display.set_alpha(a);
	if (old_surface)
		engine.display.draw_from(old_surface);
}

void Font::get_textsize_plain(const char* text, float* w, float* h)
{
	assert(text);
	assert(w);
	assert(h);

	float x = 0, y = 0;
	int maxy = 0;
	int maxx = 0;
	y += font_size * 3 / 4;

	int start = first_char;
	int end = start + num_chars;

	while (*text) {
		if (*text == '\n') {
			x = 0;
			y += font_size;
		} else if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(char_data, *text - start, &x, &y, &q, 1.0f);
			maxy = MAX(maxy, q.y1);
			maxx = MAX(maxx, q.x1);
		}
		text++;
	}
	*w = maxx;
	*h = maxy;
}

void Font::get_textsize(const char* text, float* w, float* h, int nblinesmax)
{
	assert(text);
	assert(w);
	assert(h);

	float x = 0, y = 0;
	int maxy = 0;
	int maxx = 0;
	y += font_size * 3 / 4;

	int start = first_char;
	int end = start + num_chars;
	int nblines = 0;

	const char* textend = text;
	TextState* state = push_parser();
	if (!state) {
		*w = 0;
		*h = 0;
		return;
	}
	while (parse(&state, text, textend)) {
		while (text < textend) {
			unsigned char chr = *text;
			if (chr == '\n') {
				nblines++;
				if (nblinesmax != -1 && nblines == nblinesmax) {
					goto end;
				}
				x = 0;
				y += font_size;
			} else if (chr >= start && chr < end) {
				float italic = state->italic;
				stbtt_aligned_quad q;
				stbtt_GetBakedQuad(char_data, chr - start, &x, &y, &q, state->size);
				maxy = MAX(maxy, q.y1);
				maxx = MAX(maxx, q.x1);
				x += italic;
			}
			text++;
		}
	}
end:
	pop_parser();
	*w = maxx;
	*h = maxy;
}

