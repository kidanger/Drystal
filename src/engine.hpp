#ifndef ENGINE_H
#define ENGINE_H

#include <ctime>

class Display;
class EventManager;
struct Sprite;

struct lua_State;

class Engine
{
	private:
		int target_fps;
		lua_State* L;
		time_t last_load;

	public:
		Display *display;
		EventManager *event;

		void setup(int target_fps);

		void send_globals();
		void reload();

		void loop();
		void update();

		void mouse_motion(int, int);
		void mouse_press(int, int, int);
		void event_resize(int w, int h);

		void clean_up();
		void stop();
};

#endif
