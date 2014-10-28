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

#include <lua.hpp>

#ifdef BUILD_GRAPHICS
#include "graphics/display.hpp"
#endif
#ifdef BUILD_ENABLE_STATS
#include "stats.hpp"
#endif
#include "lua_functions.hpp"

class Engine
{
private:
	unsigned long target_ms_per_frame;
	bool run;
	bool loaded;
	long unsigned last_update;

	bool update_activated;
	bool draw_activated;
	bool stats_activated;
#ifdef BUILD_ENABLE_STATS
	Stats stats;
#endif

public:
#ifdef BUILD_GRAPHICS
	Display display;
#endif
	LuaFunctions lua;

	Engine(const char* filename, unsigned int target_fps);
	~Engine();

	void load();
	void loop();
	void update();
	bool is_loaded() const
	{
		return loaded;
	}

	void toggle_update();
	void toggle_draw();
	void toggle_stats();

	void stop();

	static long unsigned get_now();
};

Engine &get_engine();
