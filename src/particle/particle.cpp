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

#include <cmath>

#include "particle.hpp"

void Particle::update(System& sys, float dt)
{
	life -= dt;

	vel += accel * dt;
	x += vel * cos(dir_angle) * dt;
	y += vel * sin(dir_angle) * dt;

	float liferatio = 1 - life / lifetime;
	if (liferatio > sys.sizes[size_state + 1].at && size_state < sys.cur_size) {
		size_state += 1;
	}
	if (liferatio > sys.colors[color_state + 1].at && color_state < sys.cur_color) {
		color_state += 1;
	}
}

