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

#ifdef DODEBUG
#define STATS
#endif

#ifdef STATS
#include "engine.hpp"

class Stats
{
public:
	Stats();
	void reset(long unsigned now);
	void add_flush(int used);
	void draw(Engine& engine);
	void compute(long unsigned now, float dt);
	inline void set_display(long unsigned now)
	{
		at_display = now;
	}
	inline void set_start(long unsigned now)
	{
		at_start = now;
	}
	inline void set_event(long unsigned now)
	{
		at_event = now;
	}
	inline void set_audio(long unsigned now)
	{
		at_audio = now;
	}
	inline void set_game(long unsigned now)
	{
		at_game = now;
	}

private:
	long unsigned started_at;
	long unsigned last;

	long unsigned at_event;
	long unsigned at_game;
	long unsigned at_display;
	long unsigned at_audio;
	long unsigned at_start;

	float event;
	float game;
	float display;
	float audio;
	float active;

	// display buffer
	int nb_flushed;
	int total_nb_flushed;
	double size_flushed;

	long unsigned total_active;
	long unsigned slept;
	int ticks_passed;
	double average_dt;
};
extern Stats stats;
#define AT(something) stats.set_##something(get_now());
#else
#define AT(something)
#endif
