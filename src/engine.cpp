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
#include <cstring>
#include <lua.hpp>
#include <sys/time.h>
#include <iostream>

#ifndef EMSCRIPTEN
#include <SDL2/SDL.h> // SDL_Delay
#endif

#include "engine.hpp"
#include "log.hpp"
#include "stats.hpp"
#include "audio/audiomanager.hpp"
#include "event/event.hpp"

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

// needed for get_engine
static Engine *engine;

#ifdef STATS
Stats stats;
#endif

Engine::Engine(const char* filename, unsigned int target_fps, bool server_mode) :
	server_mode(server_mode),
	target_ms_per_frame(1000 / target_fps),
	run(true),
	loaded(false),
	last_update(get_now()),
	update_activated(true),
	draw_activated(true),
	stats_activated(false),
	display(server_mode),
	lua(filename)
{
	engine = this;
#ifdef STATS
	stats.reset(get_now());
#endif
}

Engine::~Engine()
{
}

//
// Load & loop
//

void Engine::load()
{
	if (!display.is_available()) {
		fprintf(stderr, "[ERROR] cannot run the engine, display isn't available\n");
		return;
	}

	bool successful_load = lua.load_code();
	if (run)
		// run can be disabled before init being called
		successful_load = successful_load && lua.call_init();
	run = run && successful_load;
	loaded = true;
}

void Engine::loop()
{
#ifndef EMSCRIPTEN
	while (run) {
		unsigned long at_start = get_now();

		// update everything (event, game, display)
		update();

		// wait few millis to stay at the targeted fps value
		unsigned long ms_per_frame = (get_now() - at_start) / 1000;
		if (ms_per_frame < target_ms_per_frame) {
			long sleep_time = target_ms_per_frame - ms_per_frame;
			if (sleep_time > 0) {
				SDL_Delay(sleep_time);
			}
		}
	}
#endif
}

long unsigned Engine::get_now()
{
	// in microsecond
	struct timeval stTimeVal;
	gettimeofday(&stTimeVal, NULL);
	return stTimeVal.tv_sec * 1000000ll + stTimeVal.tv_usec;
}

void Engine::update()
{
	AT(start)
	event_update();
	AT(event)

	// check if an event provocked a stop
	if (!run)
		return;

	float dt = (get_now() - last_update) / 1000000.;
	last_update = get_now();

	audiomanager_update(dt);
	AT(audio)

	if (update_activated)
		lua.call_update(dt);
	AT(game);

	if (!server_mode) {
		if (draw_activated)
			lua.call_draw();

#ifdef STATS
		if (stats_activated)
			stats.draw(*this);
#endif

		display.flip();
		AT(display);
	}

#ifdef STATS
	if (stats_activated) {
		stats.compute(get_now(), dt);
	}
#endif
}

void Engine::stop()
{
	lua.call_atexit();
	run = false;
#ifdef EMSCRIPTEN
	emscripten_cancel_main_loop();
#endif
}

void Engine::toggle_draw()
{
	draw_activated = !draw_activated;
}
void Engine::toggle_stats()
{
	stats_activated = !stats_activated;
}

void Engine::toggle_update()
{
	update_activated = !update_activated;
}

Engine &get_engine()
{
	return *engine;
}

