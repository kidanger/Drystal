#ifndef GAME_HPP
#define GAME_HPP

class Engine;
class Display;

class GameState
{
	private:
		GameState *previous;

	public:
		GameState() {}
		GameState(GameState* prev) : previous(prev) { }

		virtual void setup(Engine&) = 0;
		virtual void preload() = 0;
		virtual void update(Engine&) = 0;
		virtual void draw(Display&) = 0;
		virtual void clean(Engine&) = 0;

		GameState* get_parent() { return previous; }
		void set_parent(GameState* game) { previous = game; }
};

#endif
