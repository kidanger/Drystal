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

#include <cassert>

#include "system.hpp"
#include "particle.hpp"
#include "engine.hpp"

System::System(int x, int y) :
	particles(NULL),
	cur_size(0), cur_color(0),
	running(false),
	size(256), used(0),
	x(x), y(y), offx(0), offy(0),
	min_direction(0), max_direction(0),
	min_lifetime(0), max_lifetime(0),
	min_initial_acceleration(0), max_initial_acceleration(0),
	min_initial_velocity(0), max_initial_velocity(0),
	emission_rate(0), emit_counter(0),
	ref(0)
{
	particles = new Particle[size];
	for (int i = 0; i < size; i++)
		particles[i].dead = true;
}

System::~System()
{
	delete[] particles;
}

void System::start()
{
	running = true;
}

void System::pause()
{
	running = false;
}

void System::stop()
{
	for (int i = 0; i < used; i++) {
		Particle* p = &particles[i];
		p->dead = true;
	}
	used = 0;
	running = false;
}

void System::draw(float dx, float dy)
{
	Engine& engine = get_engine();

	for (int i = used - 1; i >= 0; i--) {
		Particle* p = &particles[i];

		float liferatio = 1 - p->life / p->lifetime;

		float _size;
		{
			Size& sA = sizes[p->size_state];
			Size& sB = sizes[p->size_state + 1];

			float ratio = (liferatio - sA.at) / (sB.at - sA.at);

			float sizeA = p->sizeseed * (sA.max - sA.min) + sA.min;
			float sizeB = p->sizeseed * (sB.max - sB.min) + sB.min;
			_size = sizeA * (1 - ratio) + sizeB * ratio;
		}

		float r, g, b;
		{
			Color& cA = colors[p->color_state];
			Color& cB = colors[p->color_state + 1];

			float ratio = (liferatio - cA.at) / (cB.at - cA.at);

			float colrA = p->rseed * (cA.max_r - cA.min_r) + cA.min_r;
			float colrB = p->rseed * (cB.max_r - cB.min_r) + cB.min_r;
			r = colrA * (1 - ratio) + colrB * ratio;

			float colgA = p->gseed * (cA.max_g - cA.min_g) + cA.min_g;
			float colgB = p->gseed * (cB.max_g - cB.min_g) + cB.min_g;
			g = colgA * (1 - ratio) + colgB * ratio;

			float colbA = p->bseed * (cA.max_b - cA.min_b) + cA.min_b;
			float colbB = p->bseed * (cB.max_b - cB.min_b) + cB.min_b;
			b = colbA * (1 - ratio) + colbB * ratio;
		}


		engine.display.set_color(r, g, b);
		engine.display.set_point_size(_size);
		engine.display.draw_point(dx + p->x, dy + p->y);
	}
}

void System::emit()
{
	if (used == size) {
		return;
	}
	Particle* p = &particles[used];
	p->x = x + RAND(-offx, offx);
	p->y = y + RAND(-offy, offy);
	p->sizeseed = (float) rand() / RAND_MAX;
	p->rseed = (float) rand() / RAND_MAX;
	p->gseed = (float) rand() / RAND_MAX;
	p->bseed = (float) rand() / RAND_MAX;
	p->color_state = 0;
	p->size_state = 0;

	p->dir_angle = RAND(min_direction, max_direction);
	p->accel = RAND(min_initial_acceleration, max_initial_acceleration);
	p->vel = RAND(min_initial_velocity, max_initial_velocity);

	p->lifetime = RAND(min_lifetime, max_lifetime);
	p->life = p->lifetime;

	p->dead = false;

	used += 1;
}

void System::update(float dt)
{
	for (int i = 0; i < used; i++) {
		Particle* p = &particles[i];
		p->update(*this, dt);
	}

	for (int i = 0; i < used; i++) {
		Particle* p = &particles[i];
		if (p->life <= 0) {
			p->dead = true;
			particles[i] = particles[used - 1];
			used -= 1;
			i -= 1;
		}
	}

	if (running) {
		float rate = 1.0f / emission_rate;
		emit_counter += dt;
		if (emit_counter > rate && used < size) {
			emit();
			emit_counter -= rate;
		}
	}
}

void System::add_size(float at, float min, float max)
{
	int i = cur_size;
	assert(i != MAX_SIZES);

	sizes[i].at = at;
	sizes[i].min = min;
	sizes[i].max = max;
	cur_size += 1;
}

void System::add_color(float at, float min_r, float max_r, float min_g, float max_g, float min_b, float max_b)
{
	int i = cur_color;
	assert(i != MAX_COLORS);

	colors[i].at = at;
	colors[i].min_r = min_r;
	colors[i].max_r = max_r;
	colors[i].min_g = min_g;
	colors[i].max_g = max_g;
	colors[i].min_b = min_b;
	colors[i].max_b = max_b;
	cur_color += 1;
}

