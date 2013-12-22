#pragma once

#define STATS


#ifdef STATS
#include <cstdio>

#include "engine.hpp"

struct Stats {
	long unsigned started_at;

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
	long unsigned last;

	Stats() : average_dt(16.) {
	}

	void reset(long unsigned now) {
		started_at = now;
		last = now;

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

	void add_flush(int used) {
		nb_flushed += 1;
		total_nb_flushed += 1;
		size_flushed = used * 0.1 + size_flushed * 0.9;
	}

	void draw(Engine& engine) {
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
};
extern Stats stats;
#define AT(something) long unsigned at_##something = get_now();
#else
#define AT(something)
#endif
