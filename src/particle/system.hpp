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

#include <cstdlib> // random

#define RAND(a, b) (((float) rand()/RAND_MAX) * ((b) - (a)) + (a))

#define MAX_COLORS 5
struct Color {
	float at;
	float min_r, max_r;
	float min_g, max_g;
	float min_b, max_b;
};

#define MAX_SIZES 5
struct Size {
	float at;
	float min, max;
};

class Particle;
class System
{
private:
	System(const System&);
	System& operator=(const System&);

	Particle* particles;
public:

	int cur_size;
	Size sizes[MAX_SIZES];

	int cur_color;
	Color colors[MAX_COLORS];

	bool running;

	int size;
	int used;

	int x, y;
	int offx, offy;

	float min_direction, max_direction;
	float min_lifetime, max_lifetime;

	float min_initial_acceleration, max_initial_acceleration;
	float min_initial_velocity, max_initial_velocity;

	float emission_rate;
	float emit_counter;

	int ref;

	System(int x, int y);
	~System();

	void start();
	void pause();
	void stop();
	void draw(float dx, float dy);
	void emit();
	void update(float dt);
	void add_size(float at, float min, float max);
	void add_color(float at, float min_r, float max_r, float min_g, float max_g, float min_b, float max_b);
};

