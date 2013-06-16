#ifndef ENGINE_H
#define ENGINE_H

#ifndef EMSCRIPTEN
#include <ctime>
#endif

class Display;
class EventManager;
class Network;
struct Sprite;

struct lua_State;

class Engine
{
	private:
		unsigned int target_fps;
		bool run;
		lua_State* L;
#ifndef EMSCRIPTEN
		time_t last_load;
#endif
		long unsigned last_update;
		const char* filename;

	public:
		Display *display;
		EventManager *event;
		Network *net;

		void setup(const char* filename, int target_fps);

		void send_globals();
		void reload();

		void loop();
		void update();

		void mouse_motion(int, int);
		void mouse_press(int, int, int);
		void mouse_release(int, int, int);
		void key_press(const char* key_string);
		void key_release(const char* key_string);
		void event_resize(int w, int h);

		void net_recv(const char* str);
		void net_connected();
		void net_disconnected();

		void clean_up();
		void stop();
};

#endif
