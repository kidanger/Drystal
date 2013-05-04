#include <iostream>

#include "engine.hpp"

static Engine e;

int main(int argc, const char* argv[])
{
    (void) argc;
    (void) argv;

    std::cout << "Starting..." << std::endl;

    engine_t e;
    engine_setup(&e, 42);

    std::cout << "Looping..." << std::endl;

    engin_loop(&e);
    return 0;
}
