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

#include "system.hpp"

class Particle {
public:
	bool dead;

	float x, y;
	float vel;
	float accel;
	float dir_angle;

	float life, lifetime;

	int size_state;
	float sizeseed;

	int color_state;
	float rseed;
	float gseed;
	float bseed;

	void update(System& sys, float dt);
};

