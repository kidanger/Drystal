#ifndef GAME_IMPL_HPP
#define GAME_IMPL_HPP

#include "gamestate.hpp"

class Entity;

class Engine;
class Sprite;

class Game : public GameState
{
	private:
		Entity* hero;
		void set_hero_position(EventManager& event, int mousex, int mousey);

	public:
		Game() : GameState() { };
		Game(GameState* previous) : GameState(previous) { };
		void setup(Engine&);
		void preload();
		void update(Engine&);
		void draw(Display&);
		void clean(Engine&);
};

#endif
