#ifndef MENU_HPP
#define MENU_HPP

#include "gamestate.hpp"

class Entity;

class Engine;
class Sprite;
class EventManager;

class Menu : public GameState
{
	private:
		Engine* engine;
		void close_menu();
		void(*on_click)(void*, EventManager&, int, int) = [] (void* me, EventManager&, int, int) { static_cast<Menu*>(me)->close_menu(); };

	public:
		Menu() : GameState() { };

		void setup(Engine&);
		void preload();
		void update(Engine&);
		void draw(Display&);
		void clean(Engine&);
};

#endif

