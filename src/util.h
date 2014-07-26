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

#define XREALLOC(array, nmemb, need) \
	xrealloc((void **) &(array), &(nmemb), need, sizeof((array)[0]), 32)

void msleep(unsigned long milisec);
bool is_directory(const char *directory);
void *xrealloc(void **p, size_t *new_nmemb, size_t need, size_t size, unsigned min_nmemb);
bool endswith(const char *s, const char *postfix);

static inline bool startswith(const char *s, const char *prefix)
{
	if (strncmp(s, prefix, strlen(prefix)) == 0) {
		return true;
	}

	return false;
}
#ifdef __cplusplus
}
#endif

