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

#include <stdlib.h> // random

typedef struct Color Color;
typedef struct Size Size;
typedef struct System System;

#include "graphics/surface.h"
#include "particle.h"

#define RAND(a, b) (((float) rand()/RAND_MAX) * ((b) - (a)) + (a))

#define MAX_COLORS 16
struct Color {
	float at;
	unsigned char min_r, max_r;
	unsigned char min_g, max_g;
	unsigned char min_b, max_b;
};

#define MAX_SIZES 16
struct Size {
	float at;
	float min, max;
};

struct System {
	Particle* particles;

	int cur_size;
	Size sizes[MAX_SIZES];

	int cur_color;
	Color colors[MAX_COLORS];

	Surface* texture;
	bool running;

	int size;
	int used;

	float x, y;
	float offx, offy;

	float min_direction, max_direction;
	float min_lifetime, max_lifetime;

	float min_initial_acceleration, max_initial_acceleration;
	float min_initial_velocity, max_initial_velocity;

	float emission_rate;
	float emit_counter;

	int ref;
};

System *system_new(float x, float y);
void system_free(System *s);

void system_start(System *s);
void system_stop(System *s);
void system_reset(System *s);
void system_draw(System *s, float dx, float dy);
void system_emit(System *s);
void system_update(System *s, float dt);
void system_add_size(System *s, float at, float min, float max);
void system_add_color(System *s, float at, unsigned char min_r, unsigned char max_r, unsigned char min_g, unsigned char max_g, unsigned char min_b, unsigned char max_b);
void system_clear_sizes(System *s);
void system_clear_colors(System *s);
void system_set_texture(System* s, Surface* tex);

