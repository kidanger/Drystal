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
	const char* searchpath = NULL;

	// handle arguments
	{
		int i;
		const char* search_option = "--lib_path=";
		size_t size_search_option=strlen(search_option);
		for (i = 1; i < argc; i++) {
			if (!strncmp(argv[i], search_option, size_search_option)) {
				searchpath = argv[i] + size_search_option;
			} else {
				filename = argv[i];
			}
		}
	}

	Engine e(filename, 60);

#ifndef EMSCRIPTEN
	engine = &e;
	signal(SIGUSR1, reload);
#endif

	if (searchpath)
		e.lua.add_search_path(searchpath);
	e.lua.add_search_path("/usr/share/drystal");
	e.loop();

	return 0;
}

