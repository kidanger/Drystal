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
#include <cassert>

#include "storage.hpp"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#include <string>

const char* fetch(const char* key)
{
	assert(key);

	std::string js;
	js = "if (localStorage!==undefined) {localStorage['";
	js += key;
	js += "']||''} else {''}";

	const char* value = emscripten_run_script_string(js.c_str());
	return value;
}

void store(const char* key, const char* value)
{
	assert(key);
	assert(value);

	std::string js;
	js = "localStorage['";
	js += key;
	js += "'] = '";
	js += value;
	js += "';";

	emscripten_run_script(js.c_str());
}

#else
#include <cstdio>
#include <cstring>

#include "macro.hpp"

static char data[1024] = {0};

const char* fetch(_unused_ const char* key)
{
	FILE* file = fopen(".storage", "r");
	if (file == NULL)
		return "";
	fread(data, sizeof(data), 1, file);
	fclose(file);
	return data;
}

void store(_unused_ const char* key, const char* value)
{
	assert(value);

	FILE* file = fopen(".storage", "w");
	fwrite(value, strlen(value), 1, file);
	fclose(file);
}
#endif

