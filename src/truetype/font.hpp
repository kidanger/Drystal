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

struct Surface;
struct stbtt_bakedchar;

class Font
{
private:
	Surface* surface;
	float font_size;
	int first_char;
	int num_chars;
	stbtt_bakedchar* char_data;

public:
	int ref;

	~Font();
	void draw(const char* text, float x, float y, int align=1);
	void draw_plain(const char* text, float x, float y);
	void get_textsize(const char* text, float* w, float* h, int nblines=-1);
	void get_textsize_plain(const char* text, float* w, float* h);

	static Font* load(const char* filename, float size, int first_char = 32, int num_chars = 96);
};

