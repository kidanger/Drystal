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

#include <math.h>
#include <assert.h>

#include "particle.h"
#include "system.h"

void particle_update(Particle *p, System *s, float dt)
{
	assert(p);
	assert(s);

	p->life -= dt;

	p->vel += p->accel * dt;
	p->x += p->vel * cosf(p->dir_angle) * dt;
	p->y += p->vel * sinf(p->dir_angle) * dt;

	float liferatio = 1 - p->life / p->lifetime;
	if (liferatio > s->sizes[p->size_state + 1].at && p->size_state < s->cur_size) {
		p->size_state += 1;
	}
	if (liferatio > s->colors[p->color_state + 1].at && p->color_state < s->cur_color) {
		p->color_state += 1;
	}
	if (liferatio > s->alphas[p->alpha_state + 1].at && p->alpha_state < s->cur_alpha) {
		p->alpha_state += 1;
	}
}

