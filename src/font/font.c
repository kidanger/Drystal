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
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <math.h>
#include <sys/mman.h>

#include <stb_truetype.h>

#include "graphics/display.h"
#include "macro.h"
#include "font.h"
#include "parser.h"
#include "util.h"

Font* font_load(const char* filename, float size, int first_char, int num_chars)
{
	int i;
	unsigned char *pixels_colored;
	unsigned char *pixels;
	unsigned char *data = NULL;
	long filesize;

	assert(filename);

	FILE* file = fopen(filename, "rb");
	if (!file)
		return NULL;

	fseek(file, 0L, SEEK_END);
	filesize = ftell(file);
	if (filesize < 0) {
		fclose(file);
		return NULL;
	}
	fseek(file, 0L, SEEK_SET);

	data = mmap(0, filesize, PROT_READ, MAP_PRIVATE, fileno(file), 0);
	if (data == MAP_FAILED) {
		fclose(file);
		return NULL;
	}

	// TODO: compute texture size
	int w = 2048;
	int h = 2048;
	Font* font = new(Font, 1);
	font->first_char = first_char;
	font->num_chars = num_chars;
	font->char_data = new(stbtt_bakedchar, num_chars);
	font->font_size = size;

	pixels = new(unsigned char, w * h);
	stbtt_BakeFontBitmap(data, 0, size, pixels, w, h,
	                     first_char, num_chars, font->char_data);

	munmap(data, filesize);

	pixels_colored = new(unsigned char, w * h * 4);
	memset(pixels_colored, 0xff, w * h * 4);
	for (i = 0; i < w * h; i++) {
		pixels_colored[i * 4 + 3] = pixels[i];
	}
	free(pixels);

	font->surface = display_create_surface(w, h, w, h, pixels_colored);
	display_set_filter(font->surface, FILTER_NEAREST);
	font->ref = 0;

	free(pixels_colored);

	fclose(file);

	return font;
}

void font_free(Font *font)
{
	if (!font)
		return;
	display_free_surface(font->surface);
	free(font->char_data);
	free(font);
}

static inline void draw_quad(const stbtt_aligned_quad q)
{
	display_draw_quad(
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

static inline void draw_quad_fancy(const stbtt_aligned_quad q, float italic, float dx, float dy)
{
	display_draw_quad(
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

void font_draw_plain(const Font *font, const char* text, float x, float y)
{
	assert(font);
	assert(text);

	int initialx = x;
	int start = font->first_char;
	int end = start + font->num_chars;
	y += font->font_size * 3 / 4;

	Surface* old_surface = display_get_draw_from();
	display_draw_from(font->surface);
	while (*text) {
		if (*text == '\n') {
			x = initialx;
			y += font->font_size;
		} else if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(font->char_data, *text - start, &x, &y, &q, 1.0f);
			draw_quad(q);
		}
		++text;
	}
	display_draw_from(old_surface);
}

void font_draw(const Font *font, const char* text, float x, float y, Alignment align)
{
	assert(font);
	assert(text);

	const char* initial_text = text;
	int initialx = x;
	int start = font->first_char;
	int end = start + font->num_chars;
	y += font->font_size * 3 / 4;

	Surface* old_surface = display_get_draw_from();
	display_draw_from(font->surface);

	const char* textend = text;
	int r, g, b, a;
	display_get_color(&r, &g, &b);
	display_get_alpha(&a);

	TextState* state = push_parser();
	if (!state) {
		return;
	}
	state->r = r;
	state->g = g;
	state->b = b;
	state->alpha = a;
	while (parse(&state, &text, &textend)) {
		display_set_color(state->r, state->g, state->b);
		display_set_alpha(state->alpha);

		float line_width;
		float line_height;
		if (text == initial_text || (*(text - 1) != '}' && *(text - 1) != '|')) {
			font_get_textsize(font, text, &line_width, &line_height, 1);
			if (align == ALIGN_CENTER) {
				x = initialx - line_width / 2;
			} else if (align == ALIGN_RIGHT) {
				x = initialx - line_width;
			}
		}

		while (text < textend) {
			unsigned char chr = *text;
			if (chr == '\n') {
				font_get_textsize(font, text + 1, &line_width, &line_height, 1);

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
				stbtt_GetBakedQuad(font->char_data, chr - start, &x, &y, &q, state->size);
				if (state->shadow) {
					display_set_color(0, 0, 0);
					draw_quad_fancy(q, italic, state->shadow_x, state->shadow_y);
					display_set_color(state->r, state->g, state->b);
				}
				if (state->outlined) {
					display_set_color(state->outr, state->outg, state->outb);
					float f = font->font_size * 0.04f;
					draw_quad_fancy(q, italic,       -1 * f,        0 * f);
					draw_quad_fancy(q, italic,        1 * f,        0 * f);
					draw_quad_fancy(q, italic,        0 * f,       -1 * f);
					draw_quad_fancy(q, italic,        0 * f,        1 * f);
					draw_quad_fancy(q, italic, (float)  M_SQRT1_2 * f, (float)  M_SQRT1_2 * f);
					draw_quad_fancy(q, italic, (float) -M_SQRT1_2 * f, (float)  M_SQRT1_2 * f);
					draw_quad_fancy(q, italic, (float) -M_SQRT1_2 * f, (float) -M_SQRT1_2 * f);
					draw_quad_fancy(q, italic, (float)  M_SQRT1_2 * f, (float) -M_SQRT1_2 * f);
					display_set_color(state->r, state->g, state->b);
				}
				draw_quad_fancy(q, italic, 0.0, 0.0);
			}
			text++;
		}
	}
	pop_parser();
	display_set_color(r, g, b);
	display_set_alpha(a);
	display_draw_from(old_surface);
}

void font_get_textsize_plain(const Font *font, const char* text, float* w, float* h)
{
	assert(font);
	assert(text);
	assert(w);
	assert(h);

	float x = 0, y = 0;
	int maxy = 0;
	int maxx = 0;
	y += font->font_size * 3 / 4;

	int start = font->first_char;
	int end = start + font->num_chars;

	while (*text) {
		if (*text == '\n') {
			x = 0;
			y += font->font_size;
		} else if (*text >= start && *text < end) {
			stbtt_aligned_quad q;
			stbtt_GetBakedQuad(font->char_data, *text - start, &x, &y, &q, 1.0f);
			maxy = MAX(maxy, q.y1);
			maxx = MAX(maxx, q.x1);
		}
		text++;
	}
	*w = maxx;
	*h = maxy;
}

void font_get_textsize(const Font *font, const char* text, float* w, float* h, int nblinesmax)
{
	assert(font);
	assert(text);
	assert(w);
	assert(h);

	float x = 0, y = 0;
	int maxy = 0;
	int maxx = 0;
	y += font->font_size * 3 / 4;

	int start = font->first_char;
	int end = start + font->num_chars;
	int nblines = 0;

	const char* textend = text;
	TextState* state = push_parser();
	if (!state) {
		*w = 0;
		*h = 0;
		return;
	}
	while (parse(&state, &text, &textend)) {
		while (text < textend) {
			unsigned char chr = *text;
			if (chr == '\n') {
				nblines++;
				if (nblinesmax != -1 && nblines == nblinesmax) {
					goto end;
				}
				x = 0;
				y += font->font_size;
			} else if (chr >= start && chr < end) {
				float italic = state->italic;
				stbtt_aligned_quad q;
				stbtt_GetBakedQuad(font->char_data, chr - start, &x, &y, &q, state->size);
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

