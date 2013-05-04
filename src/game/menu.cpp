#include "game/menu.hpp"
#include "engine.hpp"
#include "event.hpp"

void Menu::close_menu()
{
    engine->pop_state();
}

void Menu::setup(Engine&)
{
}

void Menu::preload()
{
}

void Menu::update(Engine&)
{
}

void Menu::draw(Display&)
{
}

void Menu::clean(Engine&)
{
}

