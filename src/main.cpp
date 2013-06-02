#include "engine.hpp"

static Engine e;

int main(int argc, const char* argv[])
{
    const char* filename = "data/main.lua";
    if (argc == 2)
    {
        filename = argv[1];
    }

    e.setup(filename, 42);
    e.loop();
    return 0;
}
