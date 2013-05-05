#include "engine.hpp"

static Engine e;

int main(int argc, const char* argv[])
{
    (void) argc;
    (void) argv;

    e.setup(42);
    e.loop();
    return 0;
}
