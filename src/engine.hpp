#pragma once

#include "display.hpp"
#include "event.hpp"
#include "audio.hpp"
#include "lua_functions.hpp"
#include "storage.hpp"

struct lua_State;

class Engine
{
private:
	bool server_mode;
	unsigned long target_ms_per_frame;
	bool run;
	long unsigned last_update;

	bool update_activated;
	bool draw_activated;
	bool stats_activated;

public:
	Display display;
	EventManager event;
	Audio audio;
	LuaFunctions lua;
	Storage storage;

	Engine(const char* filename, unsigned int target_fps, bool server_mode);
	~Engine();

	void loop();
	void update();
	long unsigned get_now() const;

	void resize_event(int w, int h) const;
	void mouse_motion(int x, int y, int dx, int dy) const;
	void mouse_press(int, int, int) const;
	void mouse_release(int, int, int) const;
	void key_press(const char* key_string) const;
	void key_release(const char* key_string) const;
	void key_text(const char* string) const;

	void toggle_update();
	void toggle_draw();
	void toggle_stats();

	void stop();

};

Engine &get_engine();
#ifdef EMSCRIPTEN
#define DEFINE_EXTENSION(name) extern "C" int __attribute__((used)) luaopen_##name(lua_State *L)
#else
#define DEFINE_EXTENSION(name) extern "C" int luaopen_##name(lua_State *L)
#endif
