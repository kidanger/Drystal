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

#include <assert.h>

#include "graphics/display.h"
#include "system.h"
#include "particle.h"
#include "util.h"

System *system_new(int x, int y)
{
	System *s;

	s = new(System, 1);

	s->particles = NULL;
	s->cur_size = 0;
	s->cur_color = 0;
	s->running = false;
	s->size = 256;
	s->used = 0;
	s->x = x;
	s->y = y;
	s->offx = 0;
	s->offy = 0;
	s->min_direction = 0;
	s->max_direction = 0;
	s->min_lifetime = 0;
	s->max_lifetime = 0;
	s->min_initial_acceleration = 0;
	s->max_initial_acceleration = 0;
	s->min_initial_velocity = 0;
	s->max_initial_velocity = 0;
	s->emission_rate = 0;
	s->emit_counter = 0;
	s->ref = 0;

	s->particles = new(Particle, s->size);
	for (int i = 0; i < s->size; i++)
		s->particles[i].dead = true;

	return s;
}

void system_free(System *s)
{
	if (!s)
		return;

	free(s->particles);
	free(s);
}

void system_start(System *s)
{
	assert(s);

	s->running = true;
}

void system_pause(System *s)
{
	assert(s);

	s->running = false;
}

void system_stop(System *s)
{
	assert(s);

	for (int i = 0; i < s->used; i++)
		s->particles[i].dead = true;

	s->used = 0;
	s->running = false;
}

void system_draw(System *s, float dx, float dy)
{
	assert(s);

	for (int i = s->used - 1; i >= 0; i--) {
		Particle* p = &s->particles[i];

		float liferatio = 1 - p->life / p->lifetime;

		float _size;
		{
			Size sA = s->sizes[p->size_state];
			Size sB = s->sizes[p->size_state + 1];

			float ratio = (liferatio - sA.at) / (sB.at - sA.at);

			float sizeA = p->sizeseed * (sA.max - sA.min) + sA.min;
			float sizeB = p->sizeseed * (sB.max - sB.min) + sB.min;
			_size = sizeA * (1 - ratio) + sizeB * ratio;
		}

		float r, g, b;
		{
			Color cA = s->colors[p->color_state];
			Color cB = s->colors[p->color_state + 1];

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

		display_set_color(r, g, b);
		display_set_point_size(_size);
		display_draw_point(dx + p->x, dy + p->y);
	}
}

void system_emit(System *s)
{
	assert(s);

	if (s->used == s->size) {
		return;
	}
	Particle* p = &s->particles[s->used];
	p->x = s->x + RAND(-s->offx, s->offx);
	p->y = s->y + RAND(-s->offy, s->offy);
	p->sizeseed = (float) rand() / RAND_MAX;
	p->rseed = (float) rand() / RAND_MAX;
	p->gseed = (float) rand() / RAND_MAX;
	p->bseed = (float) rand() / RAND_MAX;
	p->color_state = 0;
	p->size_state = 0;

	p->dir_angle = RAND(s->min_direction, s->max_direction);
	p->accel = RAND(s->min_initial_acceleration, s->max_initial_acceleration);
	p->vel = RAND(s->min_initial_velocity, s->max_initial_velocity);

	p->lifetime = RAND(s->min_lifetime, s->max_lifetime);
	p->life = p->lifetime;

	p->dead = false;

	s->used += 1;
}

void system_update(System *s, float dt)
{
	assert(s);

	for (int i = 0; i < s->used; i++) {
		Particle* p = &s->particles[i];
		particle_update(p, s, dt);
	}

	for (int i = 0; i < s->used; i++) {
		Particle* p = &s->particles[i];
		if (p->life <= 0) {
			p->dead = true;
			s->particles[i] = s->particles[s->used - 1];
			s->used -= 1;
			i -= 1;
		}
	}

	if (s->running) {
		float rate = 1.0f / s->emission_rate;
		s->emit_counter += dt;
		if (s->emit_counter > rate && s->used < s->size) {
			system_emit(s);
			s->emit_counter -= rate;
		}
	}
}

void system_add_size(System *s, float at, float min, float max)
{
	assert(s);
	assert(s->cur_size != MAX_SIZES);

	s->sizes[s->cur_size].at = at;
	s->sizes[s->cur_size].min = min;
	s->sizes[s->cur_size].max = max;
	s->cur_size += 1;
}

void system_add_color(System *s, float at, float min_r, float max_r, float min_g, float max_g, float min_b, float max_b)
{
	assert(s);
	assert(s->cur_color != MAX_COLORS);

	s->colors[s->cur_color].at = at;
	s->colors[s->cur_color].min_r = min_r;
	s->colors[s->cur_color].max_r = max_r;
	s->colors[s->cur_color].min_g = min_g;
	s->colors[s->cur_color].max_g = max_g;
	s->colors[s->cur_color].min_b = min_b;
	s->colors[s->cur_color].max_b = max_b;
	s->cur_color += 1;
}

