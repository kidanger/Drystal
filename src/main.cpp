#include <iostream>

#include "engine.hpp"

static Engine e;

int main(int argc, const char* argv[])
{
    (void) argc;
    (void) argv;

    std::cout << "Starting..." << std::endl;

    e.setup(42);

    std::cout << "Looping..." << std::endl;

    e.loop();
    return 0;
}
