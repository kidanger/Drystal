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

struct TextState {
	float size;
	float italic;

	int r;
	int g;
	int b;
	int alpha;

	bool outlined;
	int outr;
	int outg;
	int outb;

	bool shadow;
	float shadow_x;
	float shadow_y;

	TextState() :
		size(1.0),
                italic(0.0),
                r(0),
                g(0),
                b(0),
		alpha(255),
                outlined(false),
                outr(0),
                outg(0),
                outb(0),
                shadow(false),
                shadow_x(0),
                shadow_y(0)
	{
	}
};

bool parse(TextState** state, const char*& text, const char*& end);
void reset_parser(int r, int g, int b, int a);

