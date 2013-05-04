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
		void event_resize(int w, int h);

		void loop();
		void update();
		void reload();
		void send_globals();

		void mouse_motion(int, int);
		void mouse_press(int, int, int);

		void draw_sprite(const Sprite&, int, int);
		void set_background(int r, int g, int b);

		void clean_up();
		void stop();

		EventManager& get_event_manager() { return *event; }
};

#endif
