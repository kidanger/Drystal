#ifndef ENGINE_H
#define ENGINE_H

#ifndef EMSCRIPTEN
#include <ctime>
#endif

#include "display.hpp"
#include "network.hpp"
#include "event.hpp"
#include "audio.hpp"
#include "lua_functions.hpp"

struct lua_State;

class Engine
{
	private:
		unsigned int target_fps;
		bool run;
		long unsigned last_update;
		long unsigned get_now() const;

	public:
		Display display;
		EventManager event;
		Network net;
		Audio audio;
		LuaFunctions lua;

		Engine(const char* filename, int target_fps);
		~Engine();

		void loop();
		void update();

		void resize_event(int w, int h) const;
		void mouse_motion(int, int) const;
		void mouse_press(int, int, int) const;
		void mouse_release(int, int, int) const;
		void key_press(const char* key_string) const;
		void key_release(const char* key_string) const;

		void receive(const char* str) const;
		void connected() const;
		void disconnected() const;

		void stop();
};

#endif
