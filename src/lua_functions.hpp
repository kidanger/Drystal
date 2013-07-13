#ifndef LUA_FUNCTIONS_H
#define LUA_FUNCTIONS_H

#include <lua.hpp>

#ifndef EMSCRIPTEN
#include <ctime>
#endif

#include "engine.hpp"

class LuaFunctions
{
	public:
		LuaFunctions(Engine&, const char *filename);
		~LuaFunctions();

		bool load_code();
		bool reload_code();

		bool call_update(double dt);
		bool call_draw();

		void call_resize_event(int w, int h) const;
		void call_mouse_motion(int mx, int my) const;
		void call_mouse_press(int mx, int my, int button) const;
		void call_mouse_release(int mx, int my, int button) const;
		void call_key_press(const char* key_string) const;
		void call_key_release(const char* key_string) const;
		void call_receive(const char* str) const;
		void call_connected() const;
		void call_disconnected() const;

	private:
		lua_State* L;
		const char* filename;
#ifndef EMSCRIPTEN
		time_t last_load;
#endif

		void send_globals() const;
};

#endif

