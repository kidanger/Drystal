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
#include "stats.hpp"

#ifdef STATS
#include "engine.hpp"
#include "display.hpp"

Stats::Stats()
	: started_at(0),
	  last(0),
	  at_event(0),
	  at_game(0),
	  at_display(0),
	  at_audio(0),
	  at_start(0),
	  event(0),
	  game(0),
	  display(0),
	  audio(0),
	  active(0),
	  nb_flushed(0),
	  total_nb_flushed(0),
	  size_flushed(0),
	  total_active(0),
	  slept(0),
	  ticks_passed(0),
	  average_dt(16.)
{
}

void Stats::reset(long unsigned now)
{
	started_at = now;
	last = now;

	at_event = 0;
	at_game = 0;
	at_display = 0;
	at_audio = 0;
	at_start = 0;

	event = 0;
	game = 0;
	display = 0;
	total_active = 0;
	slept = 0;
	ticks_passed = 0;

	nb_flushed = 0;
	total_nb_flushed = 0;
	size_flushed = 0;
}

void Stats::add_flush(int used)
{
	nb_flushed += 1;
	total_nb_flushed += 1;
	size_flushed = used * 0.1 + size_flushed * 0.9;
}

void Stats::draw(Engine& engine)
{
	float event_ratio = event / active;
	float audio_ratio = audio / active;
	float game_ratio = game / active;
	float display_ratio = display / active;
	Display& drystal = engine.display;
	int w, h;
	// this almost looks like a lua code :)
	drystal.surface_size(drystal.get_screen(), &w, &h);
	drystal.set_alpha(150);

	float wr = 40;
	float hr = h / 2;
	float xr = w - wr - 5;
	float yr = h - hr - 10;

	Camera camera = drystal.get_camera();
	drystal.reset_camera();

#define draw_rect(x, y, w, h) \
	drystal.draw_triangle((x), (y), (x)+(w), (y), (x), (y)+(h)); \
	drystal.draw_triangle((x)+(w), (y), (x)+(w), (y)+(h), (x), (y)+(h));

	float y = yr;

	drystal.set_color(0, 0, 255);
	draw_rect(xr, y, wr, hr * event_ratio);
	y += hr * event_ratio;

	drystal.set_color(255, 0, 255);
	draw_rect(xr, y, wr, hr * audio_ratio);
	y += hr * audio_ratio;

	drystal.set_color(255, 0, 0);
	draw_rect(xr, y, wr, hr * game_ratio);
	y += hr * game_ratio;

	drystal.set_color(255, 255, 0);
	draw_rect(xr, y, wr, hr * display_ratio);

	float wl = 10;
	float hl = hr;
	float xl = xr - wl - 2;
	float yl = h - hr - 10;

	drystal.set_alpha(255);
	drystal.set_color(255, 0, 0);
	float size = size_flushed / 50;
	for (int i = 0; i <= nb_flushed; i++) {
		float dy = i * (size + 2);
		draw_rect(xl, yl + dy, wl, size);
	}

	drystal.set_alpha(255);
	drystal.set_color(255, 255, 0);
	draw_rect(xl - wl - 2, yl, wl, hl);
	drystal.set_color(0, 255, 0);
	draw_rect(xl - wl - 2, yl, wl, hl * active / (slept + active));

	drystal.set_camera_angle(camera.angle);
	drystal.set_camera_position(camera.dx, camera.dy);
	drystal.set_camera_zoom(camera.zoom);

#undef draw_rect
}

void Stats::compute(long unsigned now, float dt)
{
	ticks_passed++;
	event = event * .99 + (at_event - at_start) * .01;
	audio = audio * .99 + (at_audio - at_event) * .01;
	game = game * .99 + (at_game - at_audio) * .01;
	display = display * .99 + (at_display - at_game) * .01;
	active = active * .99 + (at_display - at_start) * .01;
	total_active += at_display - at_start;

	average_dt = dt * 0.01 + average_dt * 0.99;
	slept = slept * .99 + (at_start - last) * .01;
	nb_flushed = 0;
	last = now;
}
#endif
