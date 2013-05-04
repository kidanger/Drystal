#ifndef ENGINE_H
#define ENGINE_H

#include <ctime>

class Display;
class EventManager;
class GameState;

struct lua_State;

class Engine
{
	private:
		int target_fps;
		Display *display;
		EventManager *event;
		GameState *game;
		lua_State* L;
		time_t last_load;

	public:
		void setup(int target_fps);

		void loop();
		void update();
		void push_state(GameState* game);
		void pop_state();
		void reload();
		void send_globals();

		void mouse_motion(int, int);
		void mouse_press(int, int, int);

		void clean_up();
		void stop();

		EventManager& get_event_manager() { return *event; }
};

#endif
