#include <cstring>
#include <lua.hpp>
#include <sys/time.h>
#include <iostream>

#include "log.hpp"
#include "engine.hpp"

#ifdef EMSCRIPTEN
#include "emscripten.h"
#else
#include "file.hpp"
#endif

#ifdef EMSCRIPTEN
// needed to call engine->update from emscripten library
static Engine *engine;
#endif

Engine::Engine(const char* filename, int target_fps) :
	target_fps(target_fps),
	run(true),
	last_update(get_now()),
	event(*this),
	net(*this),
	lua(*this, filename)
{
#ifdef EMSCRIPTEN
	engine = this;
#endif
}

Engine::~Engine()
{
}

//
// Main loop
//

void Engine::loop()
{
	bool successful_load = lua.load_code();
#ifdef EMSCRIPTEN
	if (successful_load)
		emscripten_set_main_loop([]() { engine->update(); }, this->target_fps, true);
#else
	run = run and successful_load;
	while (run)
	{
		unsigned long at_start = get_now();

		// update everything (network, event, game, display)
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
	net.disconnect();
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
	event.poll();
	net.poll();

#ifndef EMSCRIPTEN
	if (tick % 30 == 0)
		lua.reload_code();
#endif

	double dt = (get_now() - last_update) / 1000;
	last_update = get_now();

	lua.call_update(dt);
	lua.call_draw();

	tick += 1;
}

//
// Events
//

void Engine::resize_event(int w, int h) const
{
	lua.call_resize_event(w, h);
}

void Engine::mouse_motion(int mx, int my) const
{
	lua.call_mouse_motion(mx, my);
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

void Engine::receive(const char* str) const
{
	lua.call_receive(str);
}

void Engine::connected() const
{
	lua.call_connected();
}

void Engine::disconnected() const
{
	lua.call_disconnected();
}

void Engine::stop()
{
	run = false;
#ifdef EMSCRIPTEN
	emscripten_cancel_main_loop();
#endif
}

