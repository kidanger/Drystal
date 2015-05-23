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

#include <assert.h>
#include <stdbool.h>

typedef struct TextState TextState;

struct TextState {
	float size;
	float italic;

	int r;
	int g;
	int b;
	int alpha;

	bool outlined;
	bool shadow;

	int outr;
	int outg;
	int outb;

	float shadow_x;
	float shadow_y;
};

static inline void textstate_reset(TextState *t)
{
	assert(t);

	t->size = 1.0;
	t->italic = 0.0;
	t->r = 0;
	t->g = 0;
	t->b = 0;
	t->alpha = 255;
	t->outlined = false;
	t->outr = 0;
	t->outg = 0;
	t->outb = 0;
	t->shadow = false;
	t->shadow_x = 0;
	t->shadow_y = 0;
}

TextState* push_parser(void);
bool parse(TextState **state, const char **text, const char **end);
void pop_parser(void);

