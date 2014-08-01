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
#include <cassert>

#include "camera.hpp"

Camera::Camera() :
	dx(0), dy(0),
	zoom(1), angle(0)
{
	matrix[0] = 0;
	matrix[1] = 0;
	matrix[2] = 0;
	matrix[3] = 0;
}

void Camera::update_matrix(int width, int height)
{
	float ratio;

	assert(width > 0);
	assert(height > 0);

	ratio = (float) width / height;
	matrix[0] = cos(angle);
	matrix[1] = sin(angle) * ratio;
	matrix[2] = -sin(angle) / ratio;
	matrix[3] = cos(angle);
}

void Camera::reset()
{
	dx = 0;
	dy = 0;
	angle = 0;
	zoom = 1;
}

