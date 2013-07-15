#include <cstring>
#include <lua.hpp>
#include <sys/time.h>
#include <iostream>

#include "log.hpp"
#include "engine.hpp"

#define STATS

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

#ifdef STATS
struct Stats
{
	long unsigned started_at;

	long unsigned event;
	long unsigned net;
	long unsigned game;
	long unsigned display;

	long unsigned total_active;
	long unsigned slept;
	int ticks_passed;
	double average_dt=-1;
	long unsigned last;

	Stats(long unsigned now) :
		started_at(now),
		event(0),
		net(0),
		game(0),
		display(0),
		total_active(0),
		slept(0),
		ticks_passed(0),
		last(now)
	{}

	void report()
	{
		printf("Stats report, %d ticks\n", ticks_passed);
		printf("Active/sleep time (ms) %lu/%lu ratio=%.5f\n", total_active, slept, (float)total_active/(slept+total_active));
		printf(" dt=%.2f fps=%.2f\n", average_dt, 1000/average_dt);
		printf(" event \t= %-10lu %.1f%%\n", event, 100.*event/total_active);
		printf(" net \t= %-10lu %.1f%%\n", net, 100.*net/total_active);
		printf(" game \t= %-10lu %.1f%%\n", game, 100.*game/total_active);
		printf(" display= %-10lu %.1f%%\n", display, 100.*display/total_active);
	}
};
#define AT(something) long unsigned at_##something = get_now();
#else
#define AT(something)
#endif

void Engine::update()
{
#ifdef STATS
	static Stats stats(get_now());
#endif

	static int tick = 0;
	AT(start)
	event.poll();
	AT(event)
	net.poll();
	AT(net)

#ifndef EMSCRIPTEN
	if (tick % 30 == 0)
		lua.reload_code();
#endif

	double dt = (get_now() - last_update) / 1000;
	last_update = get_now();

	lua.call_update(dt);
	AT(game);
	lua.call_draw();
	AT(display);

	tick += 1;

#ifdef STATS
	stats.ticks_passed++;
	stats.event += at_event - at_start;
	stats.net += at_net - at_event;
	stats.game += at_game - at_net;
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

