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
#include <stdbool.h>
#include <sys/time.h>
#if defined(BUILD_EVENT) || defined(BUILD_FONT) || defined(BUILD_GRAPHICS)
#ifndef EMSCRIPTEN
#include <SDL2/SDL.h>
#else
#include <SDL/SDL.h>
#endif
#endif

#include "engine.h"
#include "log.h"
#ifdef BUILD_AUDIO
#include "audio/audio.h"
#endif
#ifdef BUILD_EVENT
#include "event/event.h"
#endif
#ifdef BUILD_GRAPHICS
#include "graphics/display.h"
#endif
#include "macro.h"
#include "util.h"

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

#include "dlua.h"

log_category("engine");

struct Engine {
	unsigned long target_ms_per_frame;
	bool run;
	bool loaded;
	long unsigned last_update;

	bool update_activated;
	bool draw_activated;
} engine;

static long unsigned get_now()
{
	// in microsecond
	struct timeval stTimeVal;
	gettimeofday(&stTimeVal, NULL);
	return stTimeVal.tv_sec * USEC_PER_SEC + stTimeVal.tv_usec;
}

void engine_init(const char* filename, unsigned int target_fps)
{
	engine.target_ms_per_frame = 1000 / target_fps;
	engine.run = true;
	engine.loaded = false;
	engine.last_update = get_now();
	engine.update_activated = true;
	engine.draw_activated = true;

	dlua_init(filename);
#ifdef BUILD_GRAPHICS
	display_init();
#endif
#if defined(BUILD_EVENT) || defined(BUILD_FONT) || defined(BUILD_GRAPHICS)
	int err = SDL_Init(0);
	if (err) {
		log_error("Cannot initialize SDL");
	}
#endif
#ifdef BUILD_EVENT
	event_init();
#endif
}

void engine_free(void)
{
	dlua_free();
#ifdef BUILD_AUDIO
	destroy_audio();
#endif
#ifdef BUILD_EVENT
	event_destroy();
#endif
#if defined(BUILD_EVENT) || defined(BUILD_FONT) || defined(BUILD_GRAPHICS)
	SDL_Quit();
#endif
}

bool engine_is_loaded(void)
{
	return engine.loaded;
}

//
// Load & loop
//

void engine_load(void)
{
#ifdef BUILD_GRAPHICS
	if (!display_is_available()) {
		log_error("Cannot run the engine, display is not available");
		return;
	}
#endif

	bool successful_load = dlua_load_code();
	if (engine.run)
		// run can be disabled before init being called
		successful_load = successful_load && dlua_call_init();
	engine.run = engine.run && successful_load;
	engine.loaded = true;
}

void engine_loop(void)
{
#ifndef EMSCRIPTEN
	while (engine.run) {
		unsigned long at_start = get_now();

		// update everything (event, game, display)
		engine_update();

		// wait few millis to stay at the targeted fps value
		unsigned long ms_per_frame = (get_now() - at_start) / 1000;
		if (ms_per_frame < engine.target_ms_per_frame) {
			unsigned long sleep_time = engine.target_ms_per_frame - ms_per_frame;
			if (sleep_time > 0) {
				msleep(sleep_time);
			}
		}
	}
#endif
}

void engine_update(void)
{
#ifdef BUILD_LIVECODING
	if (dlua_is_need_to_reload()) {
		dlua_reload_code();
	}
#endif

#ifdef BUILD_EVENT
	event_update();
#endif

	// check if an event provocked a stop
	if (!engine.run)
		return;

	float dt = (get_now() - engine.last_update) / (float) USEC_PER_SEC;
	engine.last_update = get_now();

#ifdef BUILD_AUDIO
	update_audio(dt);
#endif

	if (engine.update_activated)
		dlua_call_update(dt);

	if (engine.draw_activated)
		dlua_call_draw();

#ifdef BUILD_GRAPHICS
	display_flip();
#endif
}

void engine_stop(void)
{
	dlua_call_atexit();
	engine.run = false;
#ifdef EMSCRIPTEN
	emscripten_cancel_main_loop();
#endif
}

void engine_toggle_draw(void)
{
	engine.draw_activated = !engine.draw_activated;
}

void engine_toggle_update(void)
{
	engine.update_activated = !engine.update_activated;
}

