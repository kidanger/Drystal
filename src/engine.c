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
#include <time.h>
#ifdef BUILD_GRAPHICS
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
#ifdef BUILD_GRAPHICS
#include "event/event.h"
#include "graphics/display.h"
#endif
#include "macro.h"
#include "util.h"
#ifdef BUILD_LIVECODING
#include "livecoding.h"
#include "lua_util.h"
#endif

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
#ifdef BUILD_LIVECODING
	bool wait_next_reload;
#define QUEUE_SIZE 128
	char* files_to_reload[QUEUE_SIZE];
#endif
} engine;

static long unsigned get_now()
{
	// in microsecond
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ts.tv_sec * USEC_PER_SEC + ts.tv_nsec / NSEC_PER_USEC;
}

#ifdef BUILD_LIVECODING
static void engine_reload_queue(void);
#endif

int engine_init(const char* filename, unsigned int target_fps)
{
	engine.target_ms_per_frame = 1000 / target_fps;
	engine.run = true;
	engine.loaded = false;
	engine.last_update = get_now();
	engine.update_activated = true;
	engine.draw_activated = true;
#ifdef BUILD_LIVECODING
	engine.wait_next_reload = false;
#endif

	dlua_init(filename);
#ifdef BUILD_GRAPHICS
	int r = SDL_Init(0);
	if (r < 0) {
		log_error("Failed to initialize SDL: %s", SDL_GetError());
		return r;
	}

	r = display_init();
	if (r < 0) {
		return r;
	}

	r = event_init();
	if (r < 0) {
		return r;
	}
#endif

	return 0;
}

void engine_free(void)
{
	dlua_free();
#ifdef BUILD_AUDIO
	audio_free();
#endif
#ifdef BUILD_GRAPHICS
	event_destroy();
	display_free();
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
	bool successful_load = dlua_load_code();
	if (engine.run) {
		// run can be disabled before init being called
		if (successful_load)
			dlua_call_init();
	}

#ifdef BUILD_LIVECODING
	engine.run = engine.run && (successful_load || livecoding_is_running());
#else
	engine.run = engine.run && successful_load;
#endif

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
	float dt = (get_now() - engine.last_update) / (float) USEC_PER_SEC;
	engine.last_update = get_now();

#ifdef BUILD_LIVECODING
	if (livecoding_is_running()) {
		engine_reload_queue();
		if (engine.wait_next_reload) {
#ifdef BUILD_GRAPHICS
			event_small_update();
#endif
			return;
		}
	}
#endif

#ifdef BUILD_GRAPHICS
	event_update();
#endif

	// check if an event provocked a stop
	if (!engine.run)
		return;

#ifdef BUILD_AUDIO
	audio_update(dt);
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

#ifdef BUILD_LIVECODING
void engine_wait_next_reload(void)
{
	engine.wait_next_reload = true;
}

void engine_add_file_to_reloadqueue(const char* filename)
{
	for (int i = 0; i < QUEUE_SIZE; i++) {
		if (!engine.files_to_reload[i]) {
			engine.files_to_reload[i] = xstrdup(filename);
			break;
		}
	}
}

static void engine_reload_queue(void)
{
	lua_State* L = dlua_get_lua_state();
	assert(L);

	int top = lua_gettop(L);
	bool try_reload = false;
	for (int i = 0; i < QUEUE_SIZE; i++) {
		if (engine.files_to_reload[i]) {
			try_reload = true;
			break;
		}
	}
	if (!try_reload)
		return;

	assert_se(dlua_get_function("_reload_files"));
	lua_newtable(L);
	int idx = 1;
	for (int i = 0; i < QUEUE_SIZE; i++) {
		char* file = engine.files_to_reload[i];
		if (file) {
			lua_pushstring(L, file);
			lua_rawseti(L, -2, idx++);
			free(file);
			engine.files_to_reload[i] = NULL;
		}
	}
	call_lua_function(L, 1, 1);
	engine.wait_next_reload = !lua_toboolean(L, -1);

	if (lua_gettop(L) != top)
		lua_pop(L, 1);
	assert(lua_gettop(L) == top);
}
#endif

