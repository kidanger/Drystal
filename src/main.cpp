#include <iostream>

#include "engine.hpp"
#include "game/menu.hpp"
#include "game/gameimpl.hpp"

static Engine e;

int main(int argc, const char* argv[])
{
    (void) argc;
    (void) argv;

    Menu menu;
    Game game;
    e.setup(42);
    e.push_state(&game);
    e.push_state(&menu);
    std::cout << "Starting..." << std::endl;

    e.loop();
    return 0;
}
