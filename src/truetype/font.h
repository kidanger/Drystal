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
#pragma once

#include <stb_truetype.h>

typedef struct Font Font;

#include "graphics/surface.h"

enum Alignment {
	ALIGN_LEFT = 1,
	ALIGN_CENTER = 2,
	ALIGN_RIGHT = 3
};
typedef enum Alignment Alignment;

struct Font {
	Surface* surface;
	float font_size;
	int first_char;
	int num_chars;
	stbtt_bakedchar* char_data;

	int ref;
};

void font_free(Font *font);
void font_draw(Font *f, const char* text, float x, float y, Alignment align);
void font_draw_plain(Font *f, const char* text, float x, float y);
void font_get_textsize(Font *f, const char* text, float* w, float* h, int nblines);
void font_get_textsize_plain(Font *f, const char* text, float* w, float* h);

Font* font_load(const char* filename, float size, int first_char, int num_chars);

