/**
 * This file is part of Drystal.
 *
 * Drystal is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Drystal is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Drystal.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "display.hpp"
#include "event.hpp"
#include "audio/audio.hpp" // should be removed
#include "lua_functions.hpp"

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

	Engine(const char* filename, unsigned int target_fps, bool server_mode);
	~Engine();

	void loop();
	void update();

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

	static long unsigned get_now();
};

Engine &get_engine();
#ifdef EMSCRIPTEN
#define DEFINE_EXTENSION(name) extern "C" int __attribute__((used)) luaopen_##name(lua_State *L)
#else
#define DEFINE_EXTENSION(name) extern "C" int luaopen_##name(lua_State *L)
#endif
