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

#ifdef __cplusplus

class Engine
{
private:
	unsigned long target_ms_per_frame;
	bool run;
	bool loaded;
	long unsigned last_update;

	bool update_activated;
	bool draw_activated;

public:
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

	void stop();

	static long unsigned get_now();
};

Engine &get_engine();
extern "C" {
#endif
// only for C transition
void engine_stop(void);
void engine_toggle_update(void);
void engine_toggle_draw(void);
#ifdef __cplusplus
}
#endif

