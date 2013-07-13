#ifndef ENGINE_H
#define ENGINE_H

#ifndef EMSCRIPTEN
#include <ctime>
#endif

#include "display.hpp"
#include "network.hpp"
#include "event.hpp"
#include "audio.hpp"

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

		long unsigned get_now() const;

	public:
		Display display;
		EventManager event;
		Network net;
		Audio audio;

		Engine(const char* filename, int target_fps);
		~Engine();

		void send_globals() const;
		void load_lua();
		void reload_lua();

		void loop();
		void update();

		void mouse_motion(int, int) const;
		void mouse_press(int, int, int) const;
		void mouse_release(int, int, int) const;
		void key_press(const char* key_string) const;
		void key_release(const char* key_string) const;
		void event_resize(int w, int h) const;

		void net_recv(const char* str) const;
		void net_connected() const;
		void net_disconnected() const;

		void stop();
};

#endif
