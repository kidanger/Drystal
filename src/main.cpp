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
	if (argc == 2) {
		filename = argv[1];
	}

	Engine e(filename, 60);

#ifndef EMSCRIPTEN
	engine = &e;
	signal(SIGUSR1, reload);
#endif

	e.loop();

	return 0;
}
