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
#include <string.h>
#ifndef EMSCRIPTEN
#include <signal.h>
#endif

#include "engine.hpp"

#ifndef EMSCRIPTEN
Engine* engine;
void reload(int)
{
	engine->lua.reload_code();
}
#endif

int main(int argc, const char* argv[])
{
	const char* filename = "main.lua";
	const char* paths[8] = {NULL}; // 8 paths is enough, I guess
	int num_paths = 0;
	bool server_mode = false;

	// handle arguments
	{
		int i;
		const char* add_path_option = "--add-path=";
		size_t size_add_path_option = strlen(add_path_option);
		for (i = 1; i < argc; i++) {
			if (!strncmp(argv[i], add_path_option, size_add_path_option)) {
				paths[num_paths] = argv[i] + size_add_path_option;
				num_paths += 1;
			} else if (!strcmp(argv[i], "--server") || !strcmp(argv[i], "-s")) {
				server_mode = true;
			} else {
				filename = argv[i];
			}
		}
	}

	Engine e(filename, 60, server_mode);

#ifndef EMSCRIPTEN
	engine = &e;
	signal(SIGUSR1, reload);
#endif

	for (int i = 0; i < num_paths; i++) {
		e.lua.add_search_path(paths[i]);
	}
	e.lua.add_search_path("/usr/share/drystal");
	e.loop();

	return 0;
}

