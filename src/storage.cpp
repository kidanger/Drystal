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
#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>

#include "storage.hpp"

#ifdef EMSCRIPTEN

#include <emscripten.h>

const char* Storage::fetch(const char* key) const
{
	std::string js;
	js = "if (localStorage!==undefined) {localStorage['";
	js += key;
	js += "']||''} else {''}";

	const char* value = emscripten_run_script_string(js.c_str());
	return value;
}

void Storage::store(const char* key, const char* value)
{
	std::string js;
	js = "localStorage['";
	js += key;
	js += "'] = '";
	js += value;
	js += "';";

	emscripten_run_script(js.c_str());
}


#else

char data[1024] = {0};

const char* Storage::fetch(const char* key) const
{
	(void) key;
	FILE* file = fopen(".storage", "r");
	if (file == NULL)
		return "";
	fread(data, sizeof(data), 1, file);
	fclose(file);
	return data;
}

void Storage::store(const char* key, const char* value)
{
	(void) key;
	FILE* file = fopen(".storage", "w");
	fwrite(value, strlen(value), 1, file);
	fclose(file);
}

#endif

