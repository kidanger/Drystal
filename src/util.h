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

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "macro.h"

#define new(t, n) xmalloc(sizeof(t) * (n))
#define new0(t, n) xcalloc((n), sizeof(t))
#define newa(t, n) alloca(sizeof(t) * (n))

#define XREALLOC(array, nmemb, need) \
	xrealloc((void **) &(array), &(nmemb), need, sizeof((array)[0]), 32)

#define SWAP(a, b) { \
		typeof(a) _swaptmp = (a); \
		(a) = (b); \
		(b) = _swaptmp; \
	}

#define ANSI_HIGHLIGHT_ON        "\x1B[1;39m"
#define ANSI_HIGHLIGHT_RED_ON    "\x1B[1;31m"
#define ANSI_HIGHLIGHT_YELLOW_ON "\x1B[1;33m"
#define ANSI_HIGHLIGHT_BLUE_ON   "\x1B[1;34m"
#define ANSI_HIGHLIGHT_GRAY_ON   "\x1B[1;30m"
#define ANSI_HIGHLIGHT_PURPLE_ON "\x1B[1;35m"
#define ANSI_HIGHLIGHT_GREEN_ON  "\x1B[1;92m"
#define ANSI_RESET               "\x1B[0m"

_malloc_ void *xmalloc(size_t size);
_malloc_ void *xcalloc(size_t nmemb, size_t size);
char *xstrdup(const char *s);
int mkdir_p(const char *path);
_sentinel_ char *strjoin(const char *s, ...);
void msleep(unsigned long milisec);
bool is_directory(const char *directory);
void *xrealloc(void **p, size_t *new_nmemb, size_t need, size_t size, unsigned min_nmemb);
bool endswith(const char *s, const char *postfix);
bool on_tty(void);
bool use_colors(void);

static inline bool startswith(const char *s, const char *prefix)
{
	if (strncmp(s, prefix, strlen(prefix)) == 0) {
		return true;
	}

	return false;
}

static inline bool streq(const char *a, const char *b)
{
	return strcmp(a, b) == 0;
}

#ifdef __cplusplus
}
#endif

