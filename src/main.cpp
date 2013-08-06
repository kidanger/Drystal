#include "engine.hpp"

int main(int argc, const char* argv[])
{
	const char* filename = "main.lua";
	if (argc == 2) {
		filename = argv[1];
	}

	Engine e(filename, 60);
	e.loop();
	return 0;
}
