#include <cstring>
#include <lua.hpp>
#include <sys/time.h>
#include <iostream>

#ifndef EMSCRIPTEN
#include <SDL/SDL.h> // SDL_Delay
#endif

#include "log.hpp"
#include "engine.hpp"

#include "stats.hpp"

#ifdef EMSCRIPTEN
#include "emscripten.h"
#endif

// needed to call engine->update from emscripten library
// and for extensions (get_engine)
static Engine *engine;

#ifdef STATS
Stats stats;
#endif

Engine::Engine(const char* filename, int target_fps) :
	target_fps(target_fps),
	run(true),
	last_update(get_now()),
	update_activated(true),
	draw_activated(true),
	event(*this),
	lua(*this, filename)
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
// Main loop
//

#ifdef EMSCRIPTEN
void _engine_update()
{
	engine->update();
}
#endif
void Engine::loop()
{
	if (!display.is_available()) {
		fprintf(stderr, "[ERROR] cannot run the engine, display isn't available\n");
		return;
	}

	bool successful_load = lua.load_code();
	if (run)
		// run can be disabled before init being called
		successful_load = successful_load && lua.call_init();
#ifdef EMSCRIPTEN
	if (successful_load)
		emscripten_set_main_loop(_engine_update, 0, true);
	(void) target_fps;
#else
	run = run && successful_load;
	while (run)
	{
		unsigned long at_start = get_now();

		// update everything (event, game, display)
		update();

		// wait few millis to stay at the targeted fps value
		unsigned long now = get_now();
		if ((now - at_start)/1000 < 1000/target_fps)
		{
			long sleep_time = 1000/target_fps - (now - at_start)/1000;
			if (sleep_time > 0)
			{
				SDL_Delay(sleep_time);
			}
		}
	}
#endif
}

long unsigned Engine::get_now() const
{
	// in microsecond
	struct timeval stTimeVal;
	gettimeofday(&stTimeVal, NULL);
	return stTimeVal.tv_sec * 1000000ll + stTimeVal.tv_usec;
}

void Engine::update()
{
	static int tick = 0;
	AT(start)
	event.poll();

	// check if an event provocked a stop
	if (!run)
		return;

	AT(event)

	float dt = (get_now() - last_update) / 1000;
	last_update = get_now();

	audio.update(dt);
	AT(audio)

	if (update_activated)
		lua.call_update(dt);
	AT(game);
	if (tick % 2 && draw_activated)
		lua.call_draw();
	AT(display);

	tick += 1;

#ifdef STATS
	stats.ticks_passed++;
	stats.event += at_event - at_start;
	stats.game += at_game - at_event;
	stats.display += at_display - at_game;
	stats.total_active += at_display - at_start;

	if (stats.average_dt == -1)
		stats.average_dt = dt;
	stats.average_dt = dt*0.05 + stats.average_dt*0.95;
	stats.slept += at_start - stats.last;
	if (stats.ticks_passed % 60 == 0)
		stats.report();
	stats.last = get_now();
#endif
}

//
// Events
//

void Engine::resize_event(int w, int h) const
{
	lua.call_resize_event(w, h);
}

void Engine::mouse_motion(int x, int y, int dx, int dy) const
{
	lua.call_mouse_motion(x, y, dx, dy);
}

void Engine::mouse_press(int mx, int my, int button) const
{
	lua.call_mouse_press(mx, my, button);
}

void Engine::mouse_release(int mx, int my, int button) const
{
	lua.call_mouse_release(mx, my, button);
}

void Engine::key_press(const char* key_string) const
{
	lua.call_key_press(key_string);
}

void Engine::key_release(const char* key_string) const
{
	lua.call_key_release(key_string);
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

void Engine::toggle_update()
{
	update_activated = !update_activated;
}

Engine &get_engine()
{
	return *engine;
}

