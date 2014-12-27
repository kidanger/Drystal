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
#if defined(BUILD_FONT) || defined(BUILD_GRAPHICS)
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
#define QUEUES_SIZE 32
	char* surfaces_to_reload[QUEUES_SIZE];
	char* sounds_to_reload[QUEUES_SIZE];
#endif
} engine;

static long unsigned get_now()
{
	// in microsecond
	struct timeval stTimeVal;
	gettimeofday(&stTimeVal, NULL);
	return stTimeVal.tv_sec * USEC_PER_SEC + stTimeVal.tv_usec;
}

#ifdef BUILD_LIVECODING
static void engine_reload_queues(void);
#endif

void engine_init(const char* filename, unsigned int target_fps)
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
#if defined(BUILD_FONT) || defined(BUILD_GRAPHICS)
	int err = SDL_Init(0);
	if (err) {
		log_error("Cannot initialize SDL");
	}
#endif
#ifdef BUILD_GRAPHICS
	display_init();
	event_init();
#endif
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
#endif
#if defined(BUILD_FONT) || defined(BUILD_GRAPHICS)
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
	if (engine.run) {
		// run can be disabled before init being called
		if (successful_load)
			dlua_call_init();
	}
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
		engine.wait_next_reload = false;
		dlua_reload_code();
	}
	if (engine.wait_next_reload) {
		return;
	}
	engine_reload_queues();
#endif

#ifdef BUILD_GRAPHICS
	event_update();
#endif

	// check if an event provocked a stop
	if (!engine.run)
		return;

	float dt = (get_now() - engine.last_update) / (float) USEC_PER_SEC;
	engine.last_update = get_now();

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

#ifdef BUILD_GRAPHICS
static void reload_surface(void* data, _unused_ void* arg)
{
	Surface* s = data;
	Surface* new_surface;

	if (!s->filename)
		return;

	bool found = false;
	for (int i = 0; i < QUEUES_SIZE; i++) {
		if (engine.surfaces_to_reload[i] && endswith(s->filename, engine.surfaces_to_reload[i])) {
			free(engine.surfaces_to_reload[i]);
			engine.surfaces_to_reload[i] = NULL;
			found = true;
		}
	}
	if (!found)
		return;

	if (display_load_surface(s->filename, &new_surface))
		return;

	SWAP(s->w, new_surface->w);
	SWAP(s->h, new_surface->h);
	SWAP(s->texw, new_surface->texw);
	SWAP(s->texh, new_surface->texh);
	SWAP(s->has_fbo, new_surface->has_fbo);
	SWAP(s->has_mipmap, new_surface->has_mipmap);
	SWAP(s->npot, new_surface->npot);
	SWAP(s->tex, new_surface->tex);
	SWAP(s->fbo, new_surface->fbo);
	display_free_surface(new_surface);

	if (display_get_draw_from() == s) {
		surface_draw_from(s);
	}
	log_debug("%s reloaded", s->filename);
}

void engine_add_surface_to_reloadqueue(const char* filename)
{
	for (int i = 0; i < QUEUES_SIZE; i++) {
		if (!engine.surfaces_to_reload[i]) {
			engine.surfaces_to_reload[i] = xstrdup(filename);
			break;
		}
	}
}
#endif

#ifdef BUILD_AUDIO
static void reload_sound(void* data, _unused_ void* arg)
{
	Sound* s = data;
	Sound* new_sound;

	if (!s->filename)
		return;

	bool found = false;
	for (int i = 0; i < QUEUES_SIZE; i++) {
		if (engine.sounds_to_reload[i] && endswith(s->filename, engine.sounds_to_reload[i])) {
			free(engine.sounds_to_reload[i]);
			engine.sounds_to_reload[i] = NULL;
			found = true;
		}
	}
	if (!found)
		return;

	if (!(new_sound = sound_load_from_file(s->filename)))
		return;

	SWAP(s->alBuffer, new_sound->alBuffer);
	SWAP(s->free_me, new_sound->free_me);
	sound_free(new_sound);

	log_debug("%s reloaded", s->filename);
}

void engine_add_sound_to_reloadqueue(const char* filename)
{
	for (int i = 0; i < QUEUES_SIZE; i++) {
		if (!engine.sounds_to_reload[i]) {
			engine.sounds_to_reload[i] = xstrdup(filename);
			break;
		}
	}
}
#endif

static void engine_reload_queues(void)
{
#ifdef BUILD_GRAPHICS
	{
		bool try = false;
		for (int i = 0; i < QUEUES_SIZE; i++) {
			if (engine.surfaces_to_reload[i])
				try = true;
		}
		if (try) {
			dlua_foreach("surface", reload_surface, NULL);
		}
	}
#endif
#ifdef BUILD_AUDIO
	{
		bool try = false;
		for (int i = 0; i < QUEUES_SIZE; i++) {
			if (engine.sounds_to_reload[i])
				try = true;
		}
		if (try) {
			dlua_foreach("sound", reload_sound, NULL);
		}
	}
#endif
}

#endif

