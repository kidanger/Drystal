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
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <cassert>

#include "parser.hpp"
#include "macro.h"
#include "log.hpp"

log_category("font");

#define NB_STATES 16
#define NB_PARSERS 2
static TextState g_states[NB_PARSERS][NB_STATES];
static int g_indexes[NB_PARSERS] = {0};
static int g_indexparser = -1;
#define INDEX (g_indexes[g_indexparser])

#define START '{'
#define SEP '|'
#define END '}'


static void change_alpha(const char* str, TextState* state)
{
	assert(str);
	assert(state);

	state->alpha = atoi(str) / 100. * 255;
}

static void change_red(const char* str, TextState* state)
{
	assert(str);
	assert(state);

	state->r = atoi(str);
}
static void change_blue(const char* str, TextState* state)
{
	assert(str);
	assert(state);

	state->b = atoi(str);
}
static void change_green(const char* str, TextState* state)
{
	assert(str);
	assert(state);

	state->g = atoi(str);
}
static void change_tiny(_unused_ const char* str, TextState* state)
{
	assert(state);

	state->size = 0.6;
}
static void change_small(_unused_ const char* str, TextState* state)
{
	assert(state);

	state->size = 0.8;
}
static void change_normal(_unused_ const char* str, TextState* state)
{
	assert(state);

	state->size = 1.0;
}
static void change_big(_unused_ const char* str, TextState* state)
{
	assert(state);

	state->size = 1.3;
}
static void change_BIG(_unused_ const char* str, TextState* state)
{
	assert(state);

	state->size = 1.7;
}
static void change_italic(_unused_ const char* str, TextState* state)
{
	assert(state);

	state->italic = 3.5;
}
static void change_outline(_unused_ const char* str, TextState* state)
{
	assert(state);

	state->outlined = true;
}
static void change_nooutline(_unused_ const char* str, TextState* state)
{
	assert(state);

	state->outlined = false;
}
static void change_outr(const char* str, TextState* state)
{
	assert(str);
	assert(state);

	state->outr = atoi(str);
}
static void change_outg(const char* str, TextState* state)
{
	assert(str);
	assert(state);

	state->outg = atoi(str);
}
static void change_outb(const char* str, TextState* state)
{
	assert(str);
	assert(state);

	state->outb = atoi(str);
}
static void change_shadowx(const char* str, TextState* state)
{
	assert(str);
	assert(state);

	state->shadow_x = atoi(str);
	state->shadow = true;
}
static void change_shadowy(const char* str, TextState* state)
{
	assert(str);
	assert(state);

	state->shadow_y = atoi(str);
	state->shadow = true;
}

typedef void(*StateModifier)(const char*, TextState*);
struct Keyword {
	const char* word;
	StateModifier modifier;
};

static Keyword keywords[] = {
	{"%:",        change_alpha},
	{"r:",        change_red},
	{"g:",        change_green},
	{"b:",        change_blue},
	{"tiny",      change_tiny},
	{"small",     change_small},
	{"normal",    change_normal},
	{"big",       change_big},
	{"BIG",       change_BIG},
	{"italic",    change_italic},
	{"outline",   change_outline},
	{"nooutline", change_nooutline},
	{"outr:",     change_outr},
	{"outg:",     change_outg},
	{"outb:",     change_outb},
	{"shadowx:",  change_shadowx},
	{"shadowy:",  change_shadowy},
	{NULL, NULL}
};

static StateModifier find_modifier(const char *word)
{
	unsigned int i = 0;

	while (keywords[i].word) {
		if (strcmp(keywords[i].word, word) == 0) {
			return keywords[i].modifier;
		}
		i++;
	}
	return NULL;
}

static void evaluate(TextState* state, const char* text)
{
	assert(text);
	assert(state);

	char command[32] = {0};
	int i = 0;
	while (i < 32 && text[i] && text[i] != '|' && (i == 0 || text[i - 1] != ':')) {
		command[i] = text[i];
		i += 1;
	}

	StateModifier func = find_modifier(command);
	if (func) {
		func(text + strlen(command), state);
	} else {
		log_error("unknown command \"%s\"", command);
	}
}

static const char* next_token(const char* text)
{
	assert(text);

	while (*text) {
		if (*text == START || *text == END || *text == SEP || !*text)
			break;
		text++;
	}
	return text;
}

bool parse(TextState** state, const char*& start, const char*& end)
{
	assert(state);

	*state = &g_states[g_indexparser][INDEX];

	const char* next = next_token(start);
	if (*next == 0) {
		end = next;
	} else if (*start == END) {
		if (INDEX > 0)
			INDEX--;
		start++;
		end++;
	} else if (*next == END) {
		end = next_token(start);
	} else if (*next == START && start < next) {
		end = next;
	} else if (*next == START) {
		if (INDEX < NB_STATES - 1)
			INDEX++;
		if (INDEX > 0)
			g_states[g_indexparser][INDEX] = g_states[g_indexparser][INDEX - 1];

		do {
			start = next + 1;
			next = next_token(next + 1);
			if (*next == SEP) {
				evaluate(&g_states[g_indexparser][INDEX], start);
			}
		} while (*next != END && *next != START && *next && *(next + 1));
		end = next;

		*state = &g_states[g_indexparser][INDEX];
	} else {
		// something went wrong, moving on
		end++;
	}
	return *start != '\0';
}

TextState* push_parser()
{
	g_indexparser++;
	if (g_indexparser >= NB_PARSERS || g_indexparser < 0) {
		return NULL;
	}
	INDEX = 0;
	g_states[g_indexparser][INDEX].reset();
	return &g_states[g_indexparser][INDEX];
}

void pop_parser()
{
	g_indexparser--;
	assert(g_indexparser >= -1);
}

