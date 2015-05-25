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
#include "log.h"

log_category("system");

System *system_new(float x, float y, size_t size)
{
	System *s;

	s = new0(System, 1);

	s->x = x;
	s->y = y;
	s->size = size;

	s->particles = new(Particle, s->size);
	for (size_t i = 0; i < s->size; i++)
		s->particles[i].dead = true;

	return s;
}

System *system_clone(System* s)
{
	System *new = new(System, 1);
	memcpy(new, s, sizeof(System));

	new->particles = new(Particle, new->size);
	memcpy(new->particles, s->particles, s->size * sizeof(Particle));
	new->ref = 0;

	return new;
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

void system_stop(System *s)
{
	assert(s);

	s->running = false;
}

void system_reset(System *s)
{
	assert(s);

	for (size_t i = 0; i < s->used; i++)
		s->particles[i].dead = true;
	s->used = 0;
}

void system_draw(System *s, float dx, float dy)
{
	assert(s);

	if (!s->used)
		return;

	Surface* old_surface = display_get_draw_from();
	if (s->texture) {
		display_draw_from(s->texture);
	}

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

		unsigned char r, g, b;
		{
			Color cA = s->colors[p->color_state];
			Color cB = s->colors[p->color_state + 1];

			float ratio = (liferatio - cA.at) / (cB.at - cA.at);

			unsigned char colrA = p->rseed * (cA.max_r - cA.min_r) + cA.min_r;
			unsigned char colrB = p->rseed * (cB.max_r - cB.min_r) + cB.min_r;
			r = colrA * (1 - ratio) + colrB * ratio;

			unsigned char colgA = p->gseed * (cA.max_g - cA.min_g) + cA.min_g;
			unsigned char colgB = p->gseed * (cB.max_g - cB.min_g) + cB.min_g;
			g = colgA * (1 - ratio) + colgB * ratio;

			unsigned char colbA = p->bseed * (cA.max_b - cA.min_b) + cA.min_b;
			unsigned char colbB = p->bseed * (cB.max_b - cB.min_b) + cB.min_b;
			b = colbA * (1 - ratio) + colbB * ratio;
		}

		unsigned char alpha = 255;
		if (s->cur_alpha) {
			Alpha aA = s->alphas[p->alpha_state];
			Alpha aB = s->alphas[p->alpha_state + 1];

			float ratio = (liferatio - aA.at) / (aB.at - aA.at);

			float alphaA = p->sizeseed * (aA.max - aA.min) + aA.min;
			float alphaB = p->sizeseed * (aB.max - aB.min) + aB.min;
			alpha = alphaA * (1 - ratio) + alphaB * ratio;
		}

		display_set_color(r, g, b);
		display_set_alpha(alpha);
		if (s->texture)
			display_draw_point_tex(s->sprite_x, s->sprite_y, dx + p->x, dy + p->y, _size);
		else
			display_draw_point(dx + p->x, dy + p->y, _size);
	}

	display_draw_from(old_surface);
}

void system_emit(System *s)
{
	assert(s);

	if (s->used == s->size) {
		XREALLOC(s->particles, s->size, s->size + 1);
		log_debug("realloc upto %zu particles", s->size);
	}

	Particle* p = &s->particles[s->used];
	p->x = s->x + RAND(-s->offx, s->offx);
	p->y = s->y + RAND(-s->offy, s->offy);
	p->sizeseed = (float) rand() / RAND_MAX;
	p->rseed = (float) rand() / RAND_MAX;
	p->gseed = (float) rand() / RAND_MAX;
	p->bseed = (float) rand() / RAND_MAX;
	p->alphaseed = (float) rand() / RAND_MAX;
	p->color_state = 0;
	p->size_state = 0;
	p->alpha_state = 0;

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

	for (size_t i = 0; i < s->used; i++) {
		Particle* p = &s->particles[i];
		particle_update(p, s, dt);
	}

	for (size_t i = 0; i < s->used; i++) {
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
		if (s->emit_counter > rate) {
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

void system_add_color(System *s, float at, unsigned char min_r, unsigned char max_r, unsigned char min_g, unsigned char max_g, unsigned char min_b, unsigned char max_b)
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

void system_add_alpha(System *s, float at, float min, float max)
{
	assert(s);
	assert(s->cur_alpha != MAX_ALPHAS);

	s->alphas[s->cur_alpha].at = at;
	s->alphas[s->cur_alpha].min = min;
	s->alphas[s->cur_alpha].max = max;
	s->cur_alpha += 1;
}

void system_clear_sizes(System *s)
{
	assert(s);

	s->cur_size = 0;
}

void system_clear_colors(System *s)
{
	assert(s);

	s->cur_color = 0;
}

void system_clear_alphas(System *s)
{
	assert(s);

	s->cur_alpha = 0;
}

void system_set_texture(System* s, Surface* tex, float x, float y)
{
	s->texture = tex;
	s->sprite_x = x;
	s->sprite_y = y;
}

