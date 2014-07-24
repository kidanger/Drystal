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
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "util.h"
#include "macro.hpp"

bool endswith(const char *s, const char *postfix)
{
	size_t s_length;
	size_t p_length;

	assert(s);
	assert(postfix);

	s_length = strlen(s);
	p_length = strlen(postfix);

	if (p_length == 0)
		return true;

	if (s_length < p_length)
		return false;

	if (memcmp(s + s_length - p_length, postfix, p_length) != 0)
		return false;

	return true;
}

bool is_directory(const char *directory)
{
	struct stat s;

	assert(directory);

	if (stat(directory, &s) < 0) {
		return false;
	}

	if (S_ISDIR(s.st_mode)) {
		return true;
	}

	return false;
}

void *xrealloc(void **p, size_t *nmemb, size_t need, size_t size, unsigned min_nmemb)
{
	size_t new_totalsize;
	size_t new_nmemb;
	void *q;

	assert(p);
	assert(nmemb);
	assert(size > 0);

	if (*nmemb >= need)
		return *p;

	new_nmemb = MAX(need * 2, min_nmemb);
	new_totalsize = new_nmemb * size;

	if (new_totalsize < size * need)
		return NULL;

	q = realloc(*p, new_totalsize);
	if (!q)
		return NULL;

	*p = q;
	*nmemb = new_nmemb;
	return q;
}

