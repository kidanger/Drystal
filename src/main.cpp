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

